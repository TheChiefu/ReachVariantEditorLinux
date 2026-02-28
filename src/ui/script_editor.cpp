#include "script_editor.h"
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QMenuBar>
#include <QResource>
#include <QSaveFile>
#include "../game_variants/data/object_types.h"
#include "localized_string_editor.h"

MegaloScriptEditorWindow::MegaloScriptEditorWindow(QWidget* parent) : QDialog(parent) {
   ui.setupUi(this);
   //
   this->setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
   QObject::connect(this->ui.navigation, &QListWidget::currentItemChanged, this, [this](QListWidgetItem* current, QListWidgetItem* previous) {
      auto stack = this->ui.stack;
      if (current->text() == "Metadata Strings") {
         stack->setCurrentWidget(this->ui.pageMetadata);
         return;
      }
      if (current->text() == "All Other Strings") {
         stack->setCurrentWidget(this->ui.pageStringTable);
         return;
      }
      if (current->text() == "Forge Labels") {
         stack->setCurrentWidget(this->ui.pageForgeLabels);
         return;
      }
      if (current->text() == "Map Permissions") {
         stack->setCurrentWidget(this->ui.pageMapPerms);
         return;
      }
      if (current->text() == "Player Rating Parameters") {
         stack->setCurrentWidget(this->ui.pageRatingParams);
         return;
      }
      if (current->text() == "Required Object Types") {
         stack->setCurrentWidget(this->ui.pageReqObjectTypes);
         return;
      }
      if (current->text() == "Scripted Options") {
         stack->setCurrentWidget(this->ui.pageScriptOptions);
         return;
      }
      if (current->text() == "Scripted Player Traits") {
         stack->setCurrentWidget(this->ui.pageScriptTraits);
         return;
      }
      if (current->text() == "Scripted Stats") {
         stack->setCurrentWidget(this->ui.pageScriptStats);
         return;
      }
      if (current->text() == "Scripted HUD Widgets") {
         stack->setCurrentWidget(this->ui.pageScriptWidgets);
         return;
      }
      if (current->text() == "Gametype Code") {
         stack->setCurrentWidget(this->ui.pageScriptCode);
         return;
      }
   });
   //
   auto* menubar = new QMenuBar(this);
   this->layout()->setMenuBar(menubar);
   {
      auto* root = new QMenu(tr("File"), menubar);
      {
         auto* action = this->menu_actions.openScript = new QAction(tr("Open Script..."), root);
         QObject::connect(action, &QAction::triggered, this, &MegaloScriptEditorWindow::openScriptFromFile);
      }
      root->addAction(this->menu_actions.openScript);
      root->addSeparator();
      {
         auto* action = this->menu_actions.save = new QAction(tr("Save"), root);
         QObject::connect(action, &QAction::triggered, this, [this]() { ReachEditorState::get().saveVariant(this, false); });
      }
      {
         auto* action = this->menu_actions.saveAs = new QAction(tr("Save As..."), root);
         QObject::connect(action, &QAction::triggered, this, [this]() { ReachEditorState::get().saveVariant(this, true); });
      }
      root->addSeparator();
      {
         auto* action = this->menu_actions.saveScriptAs = new QAction(tr("Save Script As..."), root);
         QObject::connect(action, &QAction::triggered, this, &MegaloScriptEditorWindow::saveScriptAsMegalo);
      }
      root->addAction(this->menu_actions.save);
      root->addAction(this->menu_actions.saveAs);
      root->addAction(this->menu_actions.saveScriptAs);
      menubar->addAction(root->menuAction());
   }
   {
      this->menu_actions.openScript->setShortcut(QKeySequence::Open);
      this->menu_actions.save->setShortcut(QKeySequence::Save);
      this->menu_actions.saveAs->setShortcut(QKeySequence::SaveAs);
   }
   //
   auto& editor = ReachEditorState::get();
   QObject::connect(&editor, &ReachEditorState::variantFilePathChanged, this, &MegaloScriptEditorWindow::updateSaveMenuItems);
   QObject::connect(&editor, &ReachEditorState::variantAbandoned,       this, &MegaloScriptEditorWindow::updateSaveMenuItems);
   QObject::connect(&editor, &ReachEditorState::variantAcquired,        this, &MegaloScriptEditorWindow::updateSaveMenuItems);
   this->updateSaveMenuItems();
}

void MegaloScriptEditorWindow::keyPressEvent(QKeyEvent* event) {
   if (event->matches(QKeySequence::Cancel)) {
      event->ignore();
      return;
   }
   QDialog::keyPressEvent(event);
}

void MegaloScriptEditorWindow::openScriptFromFile() {
   auto* code_page = this->ui.pageContentScriptCode;
   if (!code_page)
      return;

   if (code_page->hasUnsavedEditorTextChanges()) {
      auto choice = QMessageBox::question(
         this,
         tr("Discard current script edits?"),
         tr("Opening a script file will replace the current code editor content.\n\nAny uncompiled edits in the editor will be lost. Continue?"),
         QMessageBox::StandardButton::No | QMessageBox::StandardButton::Yes,
         QMessageBox::StandardButton::No
      );
      if (choice != QMessageBox::StandardButton::Yes)
         return;
   }

   QString targetDir = QDir::currentPath();
   {
      auto path = ReachEditorState::get().variantFilePath();
      if (path && path[0]) {
         QFileInfo info(QString::fromWCharArray(path));
         if (info.exists())
            targetDir = info.absolutePath();
      }
   }

   QString fileName = QFileDialog::getOpenFileName(
      this,
      tr("Open Script"),
      targetDir,
      tr("Megalo Script (*.megalo *.txt);;All Files (*)")
   );
   if (fileName.isEmpty())
      return;

   QFile file(fileName);
   if (!file.open(QIODevice::ReadOnly)) {
      QMessageBox::information(this, tr("Unable to open file"), tr("An error occurred while trying to open this file.\n\nSystem error text:\n%1").arg(file.errorString()));
      return;
   }

   auto bytes = file.readAll();
   if (file.error() != QFileDevice::NoError) {
      QMessageBox::information(this, tr("Unable to open file"), tr("An error occurred while reading this file.\n\nSystem error text:\n%1").arg(file.errorString()));
      return;
   }

   code_page->loadCodeText(QString::fromUtf8(bytes), true);
}

void MegaloScriptEditorWindow::saveScriptAsMegalo() {
   QString script = this->ui.pageContentScriptCode->currentCodeText();

   QString targetDir = QDir::currentPath();
   QString suggested = QStringLiteral("script.megalo");
   {
      auto path = ReachEditorState::get().variantFilePath();
      if (path && path[0]) {
         QFileInfo info(QString::fromWCharArray(path));
         if (info.exists()) {
            targetDir = info.absolutePath();
            auto stem = info.completeBaseName();
            if (!stem.isEmpty())
               suggested = stem + QStringLiteral(".megalo");
         }
      }
   }

   QString fileName = QFileDialog::getSaveFileName(
      this,
      tr("Save Script As"),
      QDir(targetDir).filePath(suggested),
      tr("Megalo Script (*.megalo);;Text File (*.txt);;All Files (*)")
   );
   if (fileName.isEmpty())
      return;

   QSaveFile file(fileName);
   if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      QMessageBox::information(this, tr("Unable to save file"), tr("An error occurred while trying to save this file.\n\nSystem error text:\n%1").arg(file.errorString()));
      return;
   }

   QByteArray utf8 = script.toUtf8();
   if (file.write(utf8) != utf8.size()) {
      QMessageBox::information(this, tr("Unable to save file"), tr("An error occurred while trying to write this file.\n\nSystem error text:\n%1").arg(file.errorString()));
      file.cancelWriting();
      return;
   }

   if (!file.commit()) {
      QMessageBox::information(this, tr("Unable to save file"), tr("An error occurred while finalizing this file.\n\nSystem error text:\n%1").arg(file.errorString()));
      return;
   }
}

void MegaloScriptEditorWindow::updateSaveMenuItems() {
   auto& editor = ReachEditorState::get();
   if (!editor.variant()) {
      this->menu_actions.openScript->setEnabled(false);
      this->menu_actions.save->setEnabled(false);
      this->menu_actions.saveAs->setEnabled(false);
      this->menu_actions.saveScriptAs->setEnabled(false);
      return;
   }
   //
   bool is_resource = false;
   {
      std::wstring file = ReachEditorState::get().variantFilePath();
      if (file[0] == ':') { // Qt resource?
         is_resource = QResource(QString::fromStdWString(file)).isValid();
      }
   }
   this->menu_actions.openScript->setEnabled(true);
   this->menu_actions.save->setEnabled(!is_resource);
   this->menu_actions.saveAs->setEnabled(true);
   this->menu_actions.saveScriptAs->setEnabled(true);
}
