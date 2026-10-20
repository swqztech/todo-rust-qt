#include "main_window.h"

#include "todo_ffi.h"

#include <QDate>
#include <QApplication>
#include <QButtonGroup>
#include <QClipboard>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QVBoxLayout>

namespace {
QString todayLabel() {
  const QDate d = QDate::currentDate();
  const QStringList wd = {QStringLiteral("星期日"), QStringLiteral("星期一"),
                          QStringLiteral("星期二"), QStringLiteral("星期三"),
                          QStringLiteral("星期四"), QStringLiteral("星期五"),
                          QStringLiteral("星期六")};
  return QStringLiteral("%1月%2日 %3").arg(d.month()).arg(d.day()).arg(wd.at(d.dayOfWeek() % 7));
}
}  // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
  setWindowTitle(QStringLiteral("墨迹清单"));
  resize(520, 720);

  auto* root = new QWidget(this);
  auto* layout = new QVBoxLayout(root);

  auto* brand = new QLabel(QStringLiteral("INK LIST"));
  brand->setStyleSheet("color:#4882DC;font-weight:bold;");
  layout->addWidget(brand);
  auto* date = new QLabel(todayLabel());
  date->setStyleSheet("font-size:20px;font-weight:bold;color:#1E2834;");
  layout->addWidget(date);
  layout->addWidget(new QLabel(QStringLiteral("写下来，划掉，继续。支持导出 / 导入 / 粘贴 Markdown。")));

  auto* composer = new QHBoxLayout;
  input_ = new QLineEdit;
  input_->setPlaceholderText(QStringLiteral("新待办"));
  auto* btnAdd = new QPushButton(QStringLiteral("加入"));
  btnAdd->setStyleSheet("background:#4882DC;color:white;padding:6px 16px;");
  connect(btnAdd, &QPushButton::clicked, this, &MainWindow::onAdd);
  connect(input_, &QLineEdit::returnPressed, this, &MainWindow::onAdd);
  composer->addWidget(input_, 1);
  composer->addWidget(btnAdd);
  layout->addLayout(composer);

  auto* filterRow = new QHBoxLayout;
  auto* group = new QButtonGroup(this);
  const QStringList labels = {QStringLiteral("全部"), QStringLiteral("未完成"),
                              QStringLiteral("已完成")};
  for (int i = 0; i < labels.size(); ++i) {
    auto* b = new QPushButton(labels.at(i));
    b->setCheckable(true);
    if (i == 0) {
      b->setChecked(true);
    }
    group->addButton(b, i);
    filterRow->addWidget(b);
  }
  connect(group, &QButtonGroup::idClicked, this, &MainWindow::onFilterClicked);
  layout->addLayout(filterRow);

  auto* toolRow = new QHBoxLayout;
  auto* btnExport = new QPushButton(QStringLiteral("导出"));
  auto* btnImport = new QPushButton(QStringLiteral("导入"));
  auto* btnPaste = new QPushButton(QStringLiteral("粘贴导入"));
  connect(btnExport, &QPushButton::clicked, this, &MainWindow::onExport);
  connect(btnImport, &QPushButton::clicked, this, &MainWindow::onImportFile);
  connect(btnPaste, &QPushButton::clicked, this, &MainWindow::onPaste);
  toolRow->addWidget(btnExport);
  toolRow->addWidget(btnImport);
  toolRow->addWidget(btnPaste);
  layout->addLayout(toolRow);

  counts_ = new QLabel;
  counts_->setStyleSheet("color:#788291;font-size:12px;");
  layout->addWidget(counts_);
  layout->addWidget(new QLabel(QStringLiteral("长按条目可编辑（选中后点「编辑」）")));

  list_ = new QListWidget;
  list_->setStyleSheet("background:white;border:1px solid #D2DAE6;");
  connect(list_, &QListWidget::itemChanged, this, &MainWindow::onItemChanged);
  connect(list_, &QListWidget::itemClicked, this, &MainWindow::onItemClicked);
  layout->addWidget(list_, 1);

  auto* foot = new QHBoxLayout;
  auto* btnUp = new QPushButton(QStringLiteral("上移"));
  auto* btnDown = new QPushButton(QStringLiteral("下移"));
  auto* btnEdit = new QPushButton(QStringLiteral("编辑"));
  auto* btnDel = new QPushButton(QStringLiteral("删除"));
  btnDel->setStyleSheet("color:#C62828;");
  connect(btnUp, &QPushButton::clicked, this, &MainWindow::onMoveUp);
  connect(btnDown, &QPushButton::clicked, this, &MainWindow::onMoveDown);
  connect(btnEdit, &QPushButton::clicked, this, &MainWindow::onEditItem);
  connect(btnDel, &QPushButton::clicked, this, &MainWindow::onDelete);
  foot->addWidget(btnUp);
  foot->addWidget(btnDown);
  foot->addWidget(btnEdit);
  foot->addWidget(btnDel);
  layout->addLayout(foot);

  setCentralWidget(root);
  setStyleSheet("QMainWindow{background:#F5F8FC;}");
  refresh();
}

void MainWindow::refresh() {
  list_->blockSignals(true);
  list_->clear();
  char* json = todo_snapshot_json();
  const QByteArray raw = json ? QByteArray(json) : QByteArray("[]");
  if (json) {
    todo_free_string(json);
  }
  const QJsonArray arr = QJsonDocument::fromJson(raw).array();
  for (int i = 0; i < arr.size(); ++i) {
    const QJsonObject o = arr.at(i).toObject();
    const bool done = o.value(QStringLiteral("Done")).toBool();
    if (filter_ == FilterKind::Active && done) {
      continue;
    }
    if (filter_ == FilterKind::Done && !done) {
      continue;
    }
    const QString text = o.value(QStringLiteral("Text")).toString();
    auto* item = new QListWidgetItem(text);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(done ? Qt::Checked : Qt::Unchecked);
    item->setData(Qt::UserRole, i);
    list_->addItem(item);
  }
  list_->blockSignals(false);

  char* counts = todo_counts_label();
  if (counts) {
    counts_->setText(QString::fromUtf8(counts));
    todo_free_string(counts);
  }
}

void MainWindow::onAdd() {
  const QString t = input_->text().trimmed();
  if (t.isEmpty()) {
    return;
  }
  todo_add(t.toUtf8().constData());
  input_->clear();
  selected_list_row_ = -1;
  refresh();
}

void MainWindow::onExport() {
  char* md = todo_export_markdown();
  if (!md) {
    return;
  }
  QApplication::clipboard()->setText(QString::fromUtf8(md));
  todo_free_string(md);
  QMessageBox::information(this, QStringLiteral("墨迹清单"), QStringLiteral("已复制到剪贴板"));
}

void MainWindow::onPaste() {
  const QString text = QApplication::clipboard()->text().trimmed();
  if (text.isEmpty()) {
    QMessageBox::information(this, QStringLiteral("墨迹清单"), QStringLiteral("剪贴板为空"));
    return;
  }
  todo_import_markdown(text.toUtf8().constData());
  refresh();
  QMessageBox::information(this, QStringLiteral("墨迹清单"), QStringLiteral("已从剪贴板导入"));
}

void MainWindow::onImportFile() {
  const QString path = QFileDialog::getOpenFileName(
      this, QStringLiteral("导入"), QString(),
      QStringLiteral("Markdown/JSON (*.md *.txt *.json);;所有文件 (*.*)"));
  if (path.isEmpty()) {
    return;
  }
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return;
  }
  const QByteArray content = f.readAll();
  if (path.endsWith(QStringLiteral(".json"), Qt::CaseInsensitive)) {
    char* err = todo_import_json(content.constData());
    if (err) {
      QMessageBox::warning(this, QStringLiteral("墨迹清单"), QString::fromUtf8(err));
      todo_free_string(err);
      return;
    }
  } else {
    todo_import_markdown(content.constData());
  }
  refresh();
  QMessageBox::information(this, QStringLiteral("墨迹清单"), QStringLiteral("导入完成"));
}

void MainWindow::onFilterClicked(int id) {
  filter_ = static_cast<FilterKind>(id);
  selected_list_row_ = -1;
  refresh();
}

void MainWindow::onItemChanged(QListWidgetItem* item) {
  if (!item) {
    return;
  }
  const int storeIdx = item->data(Qt::UserRole).toInt();
  const bool done = item->checkState() == Qt::Checked;
  todo_set_done(static_cast<size_t>(storeIdx), done ? 1 : 0);
  char* counts = todo_counts_label();
  if (counts) {
    counts_->setText(QString::fromUtf8(counts));
    todo_free_string(counts);
  }
}

void MainWindow::onItemClicked(QListWidgetItem* item) {
  if (!item) {
    return;
  }
  selected_list_row_ = list_->row(item);
}

int MainWindow::selectedStoreIndex() const {
  if (selected_list_row_ < 0 || selected_list_row_ >= list_->count()) {
    return -1;
  }
  return list_->item(selected_list_row_)->data(Qt::UserRole).toInt();
}

void MainWindow::onEditItem() {
  const int idx = selectedStoreIndex();
  if (idx < 0) {
    QMessageBox::information(this, QStringLiteral("墨迹清单"), QStringLiteral("请先选中一条"));
    return;
  }
  const QString old = list_->item(selected_list_row_)->text();
  const QString text =
      QInputDialog::getText(this, QStringLiteral("编辑待办"), QStringLiteral("内容"), QLineEdit::Normal, old);
  if (text.trimmed().isEmpty()) {
    return;
  }
  todo_update_text(static_cast<size_t>(idx), text.toUtf8().constData());
  refresh();
}

void MainWindow::onDelete() {
  const int idx = selectedStoreIndex();
  if (idx < 0) {
    QMessageBox::information(this, QStringLiteral("墨迹清单"), QStringLiteral("请先选中一条"));
    return;
  }
  if (QMessageBox::question(this, QStringLiteral("墨迹清单"), QStringLiteral("删除这条待办？")) !=
      QMessageBox::Yes) {
    return;
  }
  todo_remove_at(static_cast<size_t>(idx));
  selected_list_row_ = -1;
  refresh();
}

void MainWindow::onMoveUp() {
  const int idx = selectedStoreIndex();
  if (idx < 0) {
    QMessageBox::information(this, QStringLiteral("墨迹清单"), QStringLiteral("请先选中一条"));
    return;
  }
  if (idx <= 0) {
    return;
  }
  todo_move_item(static_cast<size_t>(idx), static_cast<size_t>(idx - 1));
  refresh();
  selected_list_row_ = qMax(0, selected_list_row_ - 1);
}

void MainWindow::onMoveDown() {
  const int idx = selectedStoreIndex();
  if (idx < 0) {
    QMessageBox::information(this, QStringLiteral("墨迹清单"), QStringLiteral("请先选中一条"));
    return;
  }
  char* json = todo_snapshot_json();
  const int total = QJsonDocument::fromJson(QByteArray(json)).array().size();
  if (json) {
    todo_free_string(json);
  }
  if (idx >= total - 1) {
    return;
  }
  todo_move_item(static_cast<size_t>(idx), static_cast<size_t>(idx + 1));
  refresh();
  selected_list_row_ = qMin(list_->count() - 1, selected_list_row_ + 1);
}
