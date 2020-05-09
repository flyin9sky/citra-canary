// Copyright 2020 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QInputDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QStandardItemModel>
#include <QTimer>
#include "citra_qt/configuration/configure_touch_from_button.h"
#include "citra_qt/configuration/configure_touch_widget.h"
#include "common/param_package.h"
#include "core/3ds.h"
#include "input_common/main.h"
#include "ui_configure_touch_from_button.h"

static QString GetKeyName(int key_code) {
    switch (key_code) {
    case Qt::Key_Shift:
        return QObject::tr("Shift");
    case Qt::Key_Control:
        return QObject::tr("Ctrl");
    case Qt::Key_Alt:
        return QObject::tr("Alt");
    case Qt::Key_Meta:
        return QString{};
    default:
        return QKeySequence(key_code).toString();
    }
}

static QString ButtonToText(const Common::ParamPackage& param) {
    if (!param.Has("engine")) {
        return QObject::tr("[not set]");
    }

    if (param.Get("engine", "") == "keyboard") {
        return GetKeyName(param.Get("code", 0));
    }

    if (param.Get("engine", "") == "sdl") {
        if (param.Has("hat")) {
            const QString hat_str = QString::fromStdString(param.Get("hat", ""));
            const QString direction_str = QString::fromStdString(param.Get("direction", ""));

            return QObject::tr("Hat %1 %2").arg(hat_str, direction_str);
        }

        if (param.Has("axis")) {
            const QString axis_str = QString::fromStdString(param.Get("axis", ""));
            const QString direction_str = QString::fromStdString(param.Get("direction", ""));

            return QObject::tr("Axis %1%2").arg(axis_str, direction_str);
        }

        if (param.Has("button")) {
            const QString button_str = QString::fromStdString(param.Get("button", ""));

            return QObject::tr("Button %1").arg(button_str);
        }

        return {};
    }

    return QObject::tr("[unknown]");
}

ConfigureTouchFromButton::ConfigureTouchFromButton(
    QWidget* parent, const std::vector<Settings::TouchFromButtonMap>& touch_maps,
    const int default_index)
    : QDialog(parent), ui(std::make_unique<Ui::ConfigureTouchFromButton>()), touch_maps(touch_maps),
      selected_index(default_index), timeout_timer(std::make_unique<QTimer>()),
      poll_timer(std::make_unique<QTimer>()) {

    ui->setupUi(this);
    binding_list_model = std::make_unique<QStandardItemModel>(0, 3, this);
    binding_list_model->setHorizontalHeaderLabels({tr("Button"), tr("X"), tr("Y")});
    ui->binding_list->setModel(binding_list_model.get());
    ui->bottom_screen->SetCoordLabel(ui->coord_label);

    SetConfiguration();
    UpdateUiDisplay();
    ConnectEvents();
}

ConfigureTouchFromButton::~ConfigureTouchFromButton() = default;

void ConfigureTouchFromButton::showEvent(QShowEvent* ev) {
    QWidget::showEvent(ev);

    // width values are not valid in the constructor
    const int w =
        ui->binding_list->viewport()->contentsRect().width() / binding_list_model->columnCount();
    if (w > 0) {
        ui->binding_list->setColumnWidth(0, w);
        ui->binding_list->setColumnWidth(1, w);
        ui->binding_list->setColumnWidth(2, w);
    }
}

void ConfigureTouchFromButton::SetConfiguration() {
    for (const auto& touch_map : touch_maps) {
        ui->mapping->addItem(QString::fromStdString(touch_map.name));
    }

    ui->mapping->setCurrentIndex(selected_index);
}

void ConfigureTouchFromButton::UpdateUiDisplay() {
    ui->button_delete->setEnabled(touch_maps.size() > 1);
    ui->button_delete_bind->setEnabled(false);

    binding_list_model->removeRows(0, binding_list_model->rowCount());

    for (const auto& button_str : touch_maps[selected_index].buttons) {
        Common::ParamPackage package{button_str};
        QStandardItem* button = new QStandardItem(ButtonToText(package));
        button->setData(QString::fromStdString(button_str));
        button->setEditable(false);
        QStandardItem* xcoord = new QStandardItem(QString::number(package.Get("x", 0)));
        QStandardItem* ycoord = new QStandardItem(QString::number(package.Get("y", 0)));
        binding_list_model->appendRow({button, xcoord, ycoord});

        int dot = ui->bottom_screen->AddDot(package.Get("x", 0), package.Get("y", 0));
        button->setData(dot, data_role_dot);
    }
}

void ConfigureTouchFromButton::ConnectEvents() {
    connect(ui->mapping, qOverload<int>(&QComboBox::activated), this, [this](int index) {
        SaveCurrentMapping();
        selected_index = index;
        UpdateUiDisplay();
    });
    connect(ui->button_new, &QPushButton::clicked, this, &ConfigureTouchFromButton::NewMapping);
    connect(ui->button_delete, &QPushButton::clicked, this,
            &ConfigureTouchFromButton::DeleteMapping);
    connect(ui->button_rename, &QPushButton::clicked, this,
            &ConfigureTouchFromButton::RenameMapping);
    connect(ui->button_delete_bind, &QPushButton::clicked, this,
            &ConfigureTouchFromButton::DeleteBinding);
    connect(ui->binding_list, &QTreeView::doubleClicked, this,
            &ConfigureTouchFromButton::EditBinding);
    connect(ui->binding_list->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &ConfigureTouchFromButton::OnBindingSelection);
    connect(binding_list_model.get(), &QStandardItemModel::itemChanged, this,
            &ConfigureTouchFromButton::OnBindingChanged);
    connect(ui->binding_list->model(), &QStandardItemModel::rowsAboutToBeRemoved, this,
            &ConfigureTouchFromButton::OnBindingDeleted);
    connect(ui->bottom_screen, &TouchScreenPreview::DotAdded, this,
            &ConfigureTouchFromButton::NewBinding);
    connect(ui->bottom_screen, &TouchScreenPreview::DotSelected, this,
            &ConfigureTouchFromButton::SetActiveBinding);
    connect(ui->bottom_screen, &TouchScreenPreview::DotMoved, this,
            &ConfigureTouchFromButton::SetCoordinates);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this,
            &ConfigureTouchFromButton::ApplyConfiguration);

    connect(timeout_timer.get(), &QTimer::timeout, [this]() { SetPollingResult({}, true); });

    connect(poll_timer.get(), &QTimer::timeout, [this]() {
        Common::ParamPackage params;
        for (auto& poller : device_pollers) {
            params = poller->GetNextInput();
            if (params.Has("engine")) {
                SetPollingResult(params, false);
                return;
            }
        }
    });
}

void ConfigureTouchFromButton::SaveCurrentMapping() {
    auto& map = touch_maps[selected_index];
    map.buttons.clear();
    for (int i = 0, rc = binding_list_model->rowCount(); i < rc; ++i) {
        const auto bind_str = binding_list_model->index(i, 0)
                                  .data(Qt::ItemDataRole::UserRole + 1)
                                  .toString()
                                  .toStdString();
        if (bind_str.empty()) {
            continue;
        }
        Common::ParamPackage params{bind_str};
        if (!params.Has("engine")) {
            continue;
        }
        params.Set("x", binding_list_model->index(i, 1).data().toInt());
        params.Set("y", binding_list_model->index(i, 2).data().toInt());
        map.buttons.emplace_back(params.Serialize());
    }
}

void ConfigureTouchFromButton::NewMapping() {
    const QString name =
        QInputDialog::getText(this, tr("New Profile"), tr("Enter the name for the new profile."));
    if (name.isEmpty()) {
        return;
    }

    if (selected_index > 0) {
        SaveCurrentMapping();
    }
    touch_maps.emplace_back(Settings::TouchFromButtonMap{name.toStdString(), {}});
    selected_index = static_cast<int>(touch_maps.size()) - 1;

    ui->mapping->addItem(name);
    ui->mapping->setCurrentIndex(selected_index);
    UpdateUiDisplay();
}

void ConfigureTouchFromButton::DeleteMapping() {
    const auto answer = QMessageBox::question(
        this, tr("Delete Profile"), tr("Delete profile %1?").arg(ui->mapping->currentText()));
    if (answer != QMessageBox::Yes) {
        return;
    }
    ui->mapping->removeItem(selected_index);
    ui->mapping->setCurrentIndex(0);
    touch_maps.erase(touch_maps.begin() + selected_index);
    selected_index = touch_maps.size() ? 0 : -1;
    UpdateUiDisplay();
}

void ConfigureTouchFromButton::RenameMapping() {
    const QString new_name = QInputDialog::getText(this, tr("Rename Profile"), tr("New name:"));
    if (new_name.isEmpty()) {
        return;
    }
    ui->mapping->setItemText(selected_index, new_name);
    touch_maps[selected_index].name = new_name.toStdString();
}

void ConfigureTouchFromButton::GetButtonInput(const int row_index, const bool is_new) {
    binding_list_model->item(row_index, 0)->setText(tr("[press key]"));

    input_setter = [this, row_index, is_new](const Common::ParamPackage& params,
                                             const bool cancel) {
        auto cell = binding_list_model->item(row_index, 0);
        if (!cancel) {
            cell->setText(ButtonToText(params));
            cell->setData(QString::fromStdString(params.Serialize()));
        } else {
            if (is_new) {
                binding_list_model->removeRow(row_index);
            } else {
                cell->setText(
                    ButtonToText(Common::ParamPackage{cell->data().toString().toStdString()}));
            }
        }
    };

    device_pollers = InputCommon::Polling::GetPollers(InputCommon::Polling::DeviceType::Button);

    for (auto& poller : device_pollers) {
        poller->Start();
    }

    grabKeyboard();
    grabMouse();
    qApp->setOverrideCursor(QCursor(Qt::CursorShape::ArrowCursor));
    timeout_timer->start(5000); // Cancel after 5 seconds
    poll_timer->start(200);     // Check for new inputs every 200ms
}

void ConfigureTouchFromButton::NewBinding(const QPoint& pos) {
    QStandardItem* button = new QStandardItem();
    button->setEditable(false);
    QStandardItem* xcoord = new QStandardItem(QString::number(pos.x()));
    QStandardItem* ycoord = new QStandardItem(QString::number(pos.y()));

    int dot_id = ui->bottom_screen->AddDot(pos.x(), pos.y());
    button->setData(dot_id, data_role_dot);

    binding_list_model->appendRow({button, xcoord, ycoord});
    ui->binding_list->setFocus();
    ui->binding_list->setCurrentIndex(button->index());

    GetButtonInput(binding_list_model->rowCount() - 1, true);
}

void ConfigureTouchFromButton::EditBinding(const QModelIndex& qi) {
    if (qi.row() >= 0 && qi.column() == 0) {
        GetButtonInput(qi.row(), false);
    }
}

void ConfigureTouchFromButton::DeleteBinding() {
    const int row_index = ui->binding_list->currentIndex().row();
    if (row_index >= 0) {
        ui->bottom_screen->RemoveDot(
            binding_list_model->index(row_index, 0).data(data_role_dot).toInt());
        binding_list_model->removeRow(row_index);
    }
}

void ConfigureTouchFromButton::OnBindingSelection(const QItemSelection& selected,
                                                  const QItemSelection& deselected) {
    ui->button_delete_bind->setEnabled(!selected.isEmpty());
    if (!selected.isEmpty()) {
        const auto dot_data = selected.indexes().first().data(data_role_dot);
        if (dot_data.isValid()) {
            ui->bottom_screen->HighlightDot(dot_data.toInt());
        }
    }
    if (!deselected.isEmpty()) {
        const auto dot_data = deselected.indexes().first().data(data_role_dot);
        if (dot_data.isValid()) {
            ui->bottom_screen->HighlightDot(dot_data.toInt(), false);
        }
    }
}

void ConfigureTouchFromButton::OnBindingChanged(QStandardItem* item) {
    if (item->column() == 0) {
        return;
    }

    const bool blocked = binding_list_model->blockSignals(true);
    item->setText(QString::number(std::clamp(
        item->text().toInt(), 0,
        (item->column() == 1 ? Core::kScreenBottomWidth : Core::kScreenBottomHeight) - 1)));
    binding_list_model->blockSignals(blocked);

    const auto dot_data = binding_list_model->index(item->row(), 0).data(data_role_dot);
    if (dot_data.isValid()) {
        ui->bottom_screen->MoveDot(dot_data.toInt(),
                                   binding_list_model->item(item->row(), 1)->text().toInt(),
                                   binding_list_model->item(item->row(), 2)->text().toInt());
    }
}

void ConfigureTouchFromButton::OnBindingDeleted(const QModelIndex& parent, int first, int last) {
    for (int i = first; i <= last; ++i) {
        auto ix = binding_list_model->index(i, 0);
        if (!ix.isValid()) {
            return;
        }
        const auto dot_data = ix.data(data_role_dot);
        if (dot_data.isValid()) {
            ui->bottom_screen->RemoveDot(dot_data.toInt());
        }
    }
}

void ConfigureTouchFromButton::SetActiveBinding(const int dot_id) {
    for (int i = 0; i < binding_list_model->rowCount(); ++i) {
        if (binding_list_model->index(i, 0).data(data_role_dot) == dot_id) {
            ui->binding_list->setCurrentIndex(binding_list_model->index(i, 0));
            ui->binding_list->setFocus();
            return;
        }
    }
}

void ConfigureTouchFromButton::SetCoordinates(const int dot_id, const QPoint& pos) {
    for (int i = 0; i < binding_list_model->rowCount(); ++i) {
        if (binding_list_model->item(i, 0)->data(data_role_dot) == dot_id) {
            binding_list_model->item(i, 1)->setText(QString::number(pos.x()));
            binding_list_model->item(i, 2)->setText(QString::number(pos.y()));
            return;
        }
    }
}

void ConfigureTouchFromButton::SetPollingResult(const Common::ParamPackage& params,
                                                const bool cancel) {
    releaseKeyboard();
    releaseMouse();
    qApp->restoreOverrideCursor();
    timeout_timer->stop();
    poll_timer->stop();
    for (auto& poller : device_pollers) {
        poller->Stop();
    }
    if (input_setter) {
        (*input_setter)(params, cancel);
        input_setter.reset();
    }
}

void ConfigureTouchFromButton::keyPressEvent(QKeyEvent* event) {
    if (!input_setter && event->key() == Qt::Key_Delete) {
        DeleteBinding();
        return;
    }

    if (!input_setter) {
        return QDialog::keyPressEvent(event);
    }

    if (event->key() != Qt::Key_Escape) {
        SetPollingResult(Common::ParamPackage{InputCommon::GenerateKeyboardParam(event->key())},
                         false);
    } else {
        SetPollingResult({}, true);
    }
}

void ConfigureTouchFromButton::ApplyConfiguration() {
    SaveCurrentMapping();
    accept();
}

int ConfigureTouchFromButton::GetSelectedIndex() const {
    return selected_index;
}

std::vector<Settings::TouchFromButtonMap> ConfigureTouchFromButton::GetMaps() const {
    return touch_maps;
}

TouchScreenPreview::TouchScreenPreview(QWidget* parent) : QFrame(parent) {
    setBackgroundRole(QPalette::ColorRole::Base);
}

TouchScreenPreview::~TouchScreenPreview() = default;

void TouchScreenPreview::SetCoordLabel(QLabel* const label) {
    coord_label = label;
}

int TouchScreenPreview::AddDot(const int device_x, const int device_y) {
    QFont dot_font{QStringLiteral("monospace")};
    dot_font.setStyleHint(QFont::Monospace);
    dot_font.setPointSize(20);

    QLabel* dot = new QLabel(this);
    dot->setAttribute(Qt::WA_TranslucentBackground);
    dot->setFont(dot_font);
    dot->setText(QChar(0xD7)); // U+00D7 Multiplication Sign
    dot->setAlignment(Qt::AlignmentFlag::AlignCenter);
    dot->setProperty(prop_id, ++max_dot_id);
    dot->setProperty(prop_x, device_x);
    dot->setProperty(prop_y, device_y);
    dot->setCursor(Qt::CursorShape::PointingHandCursor);
    dot->setMouseTracking(true);
    dot->installEventFilter(this);
    dot->show();
    PositionDot(dot, device_x, device_y);
    dots.emplace_back(max_dot_id, dot);
    return max_dot_id;
}

void TouchScreenPreview::RemoveDot(const int id) {
    for (auto dot_it = dots.begin(); dot_it < dots.end(); ++dot_it) {
        if (dot_it->first == id) {
            dot_it->second->deleteLater();
            dots.erase(dot_it);
            return;
        }
    }
}

void TouchScreenPreview::HighlightDot(const int id, const bool active) const {
    for (const auto& dot : dots) {
        if (dot.first == id) {
            // use color property from the stylesheet, or fall back to the default palette
            if (dot_highlight_color.isValid()) {
                dot.second->setStyleSheet(
                    active ? QStringLiteral("color: %1").arg(dot_highlight_color.name())
                           : QString{});
            } else {
                dot.second->setForegroundRole(active ? QPalette::ColorRole::LinkVisited
                                                     : QPalette::ColorRole::NoRole);
            }
            return;
        }
    }
}

void TouchScreenPreview::MoveDot(const int id, const int device_x, const int device_y) const {
    for (const auto& dot : dots) {
        if (dot.first == id) {
            dot.second->setProperty(prop_x, device_x);
            dot.second->setProperty(prop_y, device_y);
            PositionDot(dot.second, device_x, device_y);
            return;
        }
    }
}

void TouchScreenPreview::resizeEvent(QResizeEvent* event) {
    if (ignore_resize) {
        return;
    }

    const int target_width = std::min(width(), height() * 4 / 3);
    const int target_height = std::min(height(), width() * 3 / 4);
    if (target_width == width() && target_height == height()) {
        return;
    }
    ignore_resize = true;
    setGeometry((parentWidget()->contentsRect().width() - target_width) / 2, y(), target_width,
                target_height);
    ignore_resize = false;

    if (event->oldSize().width() != target_width || event->oldSize().height() != target_height) {
        for (const auto& dot : dots) {
            PositionDot(dot.second);
        }
    }
}

void TouchScreenPreview::mouseMoveEvent(QMouseEvent* event) {
    if (!coord_label) {
        return;
    }
    const auto point = MapToDeviceCoords(event->x(), event->y());
    if (point.has_value()) {
        coord_label->setText(QStringLiteral("X: %1, Y: %2").arg(point->x()).arg(point->y()));
    } else {
        coord_label->clear();
    }
}

void TouchScreenPreview::leaveEvent(QEvent* event) {
    if (coord_label) {
        coord_label->clear();
    }
}

void TouchScreenPreview::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MouseButton::LeftButton) {
        const auto pos = MapToDeviceCoords(event->x(), event->y());
        if (pos.has_value()) {
            emit DotAdded(*pos);
        }
    }
}

bool TouchScreenPreview::eventFilter(QObject* obj, QEvent* event) {
    switch (event->type()) {
    case QEvent::Type::MouseButtonPress: {
        const auto mouse_event = static_cast<QMouseEvent*>(event);
        if (mouse_event->button() != Qt::MouseButton::LeftButton) {
            break;
        }
        emit DotSelected(obj->property(prop_id).toInt());

        drag_state.dot = qobject_cast<QLabel*>(obj);
        drag_state.start_pos = mouse_event->globalPos();
        return true;
    }
    case QEvent::Type::MouseMove: {
        if (!drag_state.dot) {
            break;
        }
        const auto mouse_event = static_cast<QMouseEvent*>(event);
        if (!drag_state.active) {
            drag_state.active =
                (mouse_event->globalPos() - drag_state.start_pos).manhattanLength() >=
                QApplication::startDragDistance();
            if (!drag_state.active) {
                break;
            }
        }
        auto current_pos = mapFromGlobal(mouse_event->globalPos());
        current_pos.setX(std::clamp(current_pos.x(), contentsMargins().left(),
                                    contentsMargins().left() + contentsRect().width()));
        current_pos.setY(std::clamp(current_pos.y(), contentsMargins().top(),
                                    contentsMargins().top() + contentsRect().height()));
        const auto device_coord = MapToDeviceCoords(current_pos.x(), current_pos.y());
        if (device_coord.has_value()) {
            drag_state.dot->setProperty(prop_x, device_coord->x());
            drag_state.dot->setProperty(prop_y, device_coord->y());
            PositionDot(drag_state.dot, device_coord->x(), device_coord->y());
            emit DotMoved(drag_state.dot->property(prop_id).toInt(), *device_coord);
            if (coord_label) {
                coord_label->setText(
                    QStringLiteral("X: %1, Y: %2").arg(device_coord->x()).arg(device_coord->y()));
            }
        }
        return true;
    }
    case QEvent::Type::MouseButtonRelease: {
        drag_state.dot.clear();
        drag_state.active = false;
        return true;
    }
    default:
        break;
    }
    return obj->eventFilter(obj, event);
}

std::optional<QPoint> TouchScreenPreview::MapToDeviceCoords(const int screen_x,
                                                            const int screen_y) const {
    const float t_x = 0.5f + static_cast<float>(screen_x - contentsMargins().left()) *
                                 (Core::kScreenBottomWidth - 1) / contentsRect().width();
    const float t_y = 0.5f + static_cast<float>(screen_y - contentsMargins().top()) *
                                 (Core::kScreenBottomHeight - 1) / contentsRect().height();
    if (t_x >= 0.5f && t_x < Core::kScreenBottomWidth && t_y >= 0.5f &&
        t_y < Core::kScreenBottomHeight) {

        return QPoint{static_cast<int>(t_x), static_cast<int>(t_y)};
    }
    return std::nullopt;
}

void TouchScreenPreview::PositionDot(QLabel* const dot, const int device_x,
                                     const int device_y) const {
    dot->move(static_cast<int>(
                  static_cast<float>(device_x >= 0 ? device_x : dot->property(prop_x).toInt()) *
                      (contentsRect().width() - 1) / (Core::kScreenBottomWidth - 1) +
                  contentsMargins().left() - static_cast<float>(dot->width()) / 2 + 0.5f),
              static_cast<int>(
                  static_cast<float>(device_y >= 0 ? device_y : dot->property(prop_y).toInt()) *
                      (contentsRect().height() - 1) / (Core::kScreenBottomHeight - 1) +
                  contentsMargins().top() - static_cast<float>(dot->height()) / 2 + 0.5f));
}