// Copyright 2020 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <vector>
#include <QDialog>
#include "core/settings.h"

class QItemSelection;
class QModelIndex;
class QStandardItemModel;
class QStandardItem;
class QTimer;

namespace Common {
class ParamPackage;
}

namespace InputCommon {
namespace Polling {
class DevicePoller;
}
} // namespace InputCommon

namespace Ui {
class ConfigureTouchFromButton;
}

class ConfigureTouchFromButton : public QDialog {
    Q_OBJECT

public:
    explicit ConfigureTouchFromButton(QWidget* parent,
                                      const std::vector<Settings::TouchFromButtonMap>& touch_maps,
                                      const int default_index = 0);
    ~ConfigureTouchFromButton() override;

    int GetSelectedIndex() const;
    std::vector<Settings::TouchFromButtonMap> GetMaps() const;

public slots:
    void ApplyConfiguration();
    void NewBinding(const QPoint& pos);
    void SetActiveBinding(const int dot_id);
    void SetCoordinates(const int dot_id, const QPoint& pos);

protected:
    virtual void showEvent(QShowEvent* ev) override;
    virtual void keyPressEvent(QKeyEvent* event) override;

private slots:
    void NewMapping();
    void DeleteMapping();
    void RenameMapping();
    void EditBinding(const QModelIndex& qi);
    void DeleteBinding();
    void OnBindingSelection(const QItemSelection& selected, const QItemSelection& deselected);
    void OnBindingChanged(QStandardItem* item);
    void OnBindingDeleted(const QModelIndex& parent, int first, int last);

private:
    void SetConfiguration();
    void UpdateUiDisplay();
    void ConnectEvents();
    void GetButtonInput(const int row_index, const bool is_new);
    void SetPollingResult(const Common::ParamPackage& params, const bool cancel);
    void SaveCurrentMapping();

    std::unique_ptr<Ui::ConfigureTouchFromButton> ui;
    std::unique_ptr<QStandardItemModel> binding_list_model;
    std::vector<Settings::TouchFromButtonMap> touch_maps;
    int selected_index;

    std::unique_ptr<QTimer> timeout_timer;
    std::unique_ptr<QTimer> poll_timer;
    std::vector<std::unique_ptr<InputCommon::Polling::DevicePoller>> device_pollers;
    std::optional<std::function<void(const Common::ParamPackage&, const bool)>> input_setter;

    static constexpr int data_role_dot = Qt::ItemDataRole::UserRole + 2;
};