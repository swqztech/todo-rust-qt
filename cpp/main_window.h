#pragma once

#include <QMainWindow>

class QListWidgetItem;
class QLineEdit;
class QListWidget;
class QLabel;
class QButtonGroup;

enum class FilterKind { All, Active, Done };

class MainWindow : public QMainWindow {
  Q_OBJECT
 public:
  explicit MainWindow(QWidget* parent = nullptr);

 private slots:
  void onAdd();
  void onExport();
  void onImportFile();
  void onPaste();
  void onFilterClicked(int id);
  void onItemChanged(QListWidgetItem* item);
  void onItemClicked(QListWidgetItem* item);
  void onEditItem();
  void onDelete();
  void onMoveUp();
  void onMoveDown();

 private:
  void refresh();
  int selectedStoreIndex() const;
  FilterKind filter_ = FilterKind::All;
  QListWidget* list_ = nullptr;
  QLineEdit* input_ = nullptr;
  QLabel* counts_ = nullptr;
  int selected_list_row_ = -1;
};
