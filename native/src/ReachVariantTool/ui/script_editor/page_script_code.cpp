#include "page_script_code.h"
#include <array>
#include "compiler_unresolved_strings.h"
#include <QCompleter>
#include <QKeyEvent>
#include <QMessageBox>
#include <QListWidget>
#include <QScrollBar>
#include <QShortcut>
#include <QStringListModel>
#include <QTextBlock>
#include <QTimer>
#include "../../helpers/qt/color.h"
#include "../generic/MegaloSyntaxHighlighter.h"
#include "../../services/ini.h"

namespace {
   static const QString ce_icon_success = ":/ScriptEditor/compiler_log/success.png";
   static const QString ce_icon_notice  = ":/ScriptEditor/compiler_log/notice.png";
   static const QString ce_icon_warning = ":/ScriptEditor/compiler_log/warning.png";
   static const QString ce_icon_error   = ":/ScriptEditor/compiler_log/error.png";
   static const QString ce_icon_fatal   = ":/ScriptEditor/compiler_log/fatal.png";
   //
   enum class _icon_type {
      unknown = 0,
      success,
      notice,
      warning,
      error,
      fatal,
   };
   constexpr int role_offset = Qt::ItemDataRole::UserRole + 0;
   constexpr int role_line   = Qt::ItemDataRole::UserRole + 1;
   constexpr int role_col    = Qt::ItemDataRole::UserRole + 2;
   constexpr int role_icon   = Qt::ItemDataRole::UserRole + 3;
}

namespace {
   const std::array ini_settings = {
      &ReachINI::CodeEditor::bOverrideBackColor,
      &ReachINI::CodeEditor::bOverrideTextColor,
      &ReachINI::CodeEditor::sBackColor,
      &ReachINI::CodeEditor::sTextColor,
      &ReachINI::CodeEditor::sFontFamily,
   };
}

ScriptEditorPageScriptCode::ScriptEditorPageScriptCode(QWidget* parent) : QWidget(parent) {
   ui.setupUi(this);
   {
      new MegaloSyntaxHighlighter(this->ui.textEditor->document());
      this->updateCodeEditorStyle();
      this->setupAutocomplete();
      QObject::connect(&ReachINI::getForQt(), &cobb::qt::ini::file::settingChanged, this, [this](cobb::ini::setting* setting, cobb::ini::setting_value_union oldValue, cobb::ini::setting_value_union newValue) {
         for (auto* ptr : ini_settings) {
            if (setting == ptr) {
               this->updateCodeEditorStyle();
               return;
            }
         }
      });
   }
   //
   auto& editor = ReachEditorState::get();
   //
   QObject::connect(this->ui.buttonDecompile, &QPushButton::clicked, [this]() {
      auto& editor  = ReachEditorState::get();
      auto  variant = editor.variant();
      if (!variant)
         return;
      auto mp = variant->get_multiplayer_data();
      if (!mp)
         return;
      Megalo::Decompiler decompiler(*variant);
      decompiler.decompile();
      //
      const QSignalBlocker blocker(this->ui.textEditor);
      this->ui.textEditor->setPlainText(decompiler.current_content);
   });
   QObject::connect(this->ui.buttonCompile, &QPushButton::clicked, [this]() {
      auto& editor = ReachEditorState::get();
      auto  variant = editor.variant();
      if (!variant)
         return;
      auto mp = variant->get_multiplayer_data();
      if (!mp)
         return;
      auto code = this->ui.textEditor->toPlainText();
      if (code.trimmed().isEmpty()) {
         if (!mp->scriptContent.triggers.empty()) {
            auto choice = QMessageBox::question(this, tr("Are you sure?"), tr("This game variant currently has script content. Compiling an empty variant will clear all script data. Are you sure you wish to proceed?"), QMessageBox::StandardButton::No | QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No);
            if (choice == QMessageBox::StandardButton::No)
               return;
         }
      }
      //
      Megalo::Compiler compiler(*mp);
      compiler.parse(code);
      //
      this->updateLog(compiler);
      if (!compiler.has_errors()) {
         auto& unresolved_str = compiler.get_unresolved_string_references();
         if (unresolved_str.size()) {
            if (!CompilerUnresolvedStringsDialog::handleCompiler(this, compiler)) {
               QMessageBox::information(this, tr("Unable to commit the compiled data"), tr("Unresolved string references were not handled."));
               this->_lastFatals.emplace_back(tr("Unable to commit the compiled data. Unresolved string references were not handled."), pos_t(-1, -1, -1));
               this->redrawLog();
               return;
            }
         }
         compiler.apply();
         emit ReachEditorState::get().variantRecompiled(variant);
         QMessageBox::information(this, tr("Compiled contents applied!"), tr("The compiled content has been applied."));
      }
   });
   QObject::connect(this->ui.compileLogFilterWarn, &QAbstractButton::toggled, [this](bool checked) {
      this->redrawLog();
   });
   QObject::connect(this->ui.compileLogFilterError, &QAbstractButton::toggled, [this](bool checked) {
      this->redrawLog();
   });
   QObject::connect(this->ui.buttonCompileLogJump, &QPushButton::clicked, [this]() {
      auto* item = this->ui.compileLog->currentItem();
      if (!item)
         return;
      this->jumpToLogItem(*item);
   });
   QObject::connect(this->ui.buttonCopyCompileLog, &QPushButton::clicked, [this]() {
      this->ui.compileLog->copyAllToClipboard();
   });
   this->ui.compileLog->setCopyTransformFunctor([](QString& out, QListWidgetItem* item) {
      switch ((_icon_type)item->data(role_icon).toInt()) {
         case _icon_type::success:
            out.prepend("[GOOD!] ");
            break;
         case _icon_type::notice:
            out.prepend("[NOTE:] ");
            break;
         case _icon_type::warning:
            out.prepend("[WARN!] ");
            break;
         case _icon_type::error:
            out.prepend("[ERROR] ");
            break;
         case _icon_type::fatal:
            out.prepend("[FATAL] ");
            break;
      }
      //
      int line = item->data(role_line).toInt();
      int col  = item->data(role_col).toInt();
      if (line < 1 || col < 0)
         return;
      out.prepend(QString("Line %1 col %2: ").arg(line).arg(col));
   });
   QObject::connect(this->ui.compileLog, &QListWidget::itemDoubleClicked, [this](QListWidgetItem* item) {
      if (!item)
         return;
      this->jumpToLogItem(*item);
   });
}
void ScriptEditorPageScriptCode::updateLog(Compiler& compiler) {
   this->_lastNotices  = compiler.get_notices();
   this->_lastWarnings = compiler.get_warnings();
   this->_lastErrors   = compiler.get_non_fatal_errors();
   this->_lastFatals   = compiler.get_fatal_errors();
   //
   this->redrawLog();
}
void ScriptEditorPageScriptCode::redrawLog() {
   auto widget = this->ui.compileLog;
   //
   const QSignalBlocker blocker(widget);
   widget->clear();
   //
   auto ico_success = QIcon(ce_icon_success);
   auto ico_notice  = QIcon(ce_icon_notice);
   auto ico_warning = QIcon(ce_icon_warning);
   auto ico_error   = QIcon(ce_icon_error);
   auto ico_fatal   = QIcon(ce_icon_fatal);
   //
   for (auto& entry : this->_lastNotices) {
      auto item = new QListWidgetItem;
      item->setText(entry.text);
      item->setData(role_offset, entry.location.offset);
      item->setData(role_line,   entry.location.line + 1);
      item->setData(role_col,    entry.location.col());
      item->setData(role_icon,   (int)_icon_type::notice);
      item->setIcon(ico_notice);
      widget->addItem(item);
   }
   if (this->ui.compileLogFilterWarn->isChecked()) {
      for (auto& entry : this->_lastWarnings) {
         auto item = new QListWidgetItem;
         item->setText(entry.text);
         item->setData(role_offset, entry.location.offset);
         item->setData(role_line,   entry.location.line + 1);
         item->setData(role_col,    entry.location.col());
         item->setData(role_icon,   (int)_icon_type::warning);
         item->setIcon(ico_warning);
         widget->addItem(item);
      }
   }
   if (this->ui.compileLogFilterError->isChecked()) {
      for (auto& entry : this->_lastErrors) {
         auto item = new QListWidgetItem;
         item->setText(entry.text);
         item->setData(role_offset, entry.location.offset);
         item->setData(role_line,   entry.location.line + 1);
         item->setData(role_col,    entry.location.col());
         item->setData(role_icon,   (int)_icon_type::error);
         item->setIcon(ico_error);
         widget->addItem(item);
      }
      for (auto& entry : this->_lastFatals) {
         auto item = new QListWidgetItem;
         item->setText(entry.text);
         item->setData(role_offset, entry.location.offset);
         item->setData(role_line,   entry.location.line + 1);
         item->setData(role_col,    entry.location.col());
         item->setData(role_icon,   (int)_icon_type::fatal);
         item->setIcon(ico_fatal);
         widget->addItem(item);
      }
   }
   if (!this->_lastErrors.size() && !this->_lastFatals.size()) {
      auto item = new QListWidgetItem;
      item->setText(tr("The script is valid and contains no errors!"));
      item->setData(role_offset, -1);
      item->setData(role_line,   -1);
      item->setData(role_col,    -1);
      item->setData(role_icon,   (int)_icon_type::success);
      item->setIcon(ico_success);
      widget->addItem(item);
   }
}

void ScriptEditorPageScriptCode::jumpToLogItem(QListWidgetItem& item) {
   int line = item.data(role_line).toInt();
   int col  = item.data(role_col).toInt();
   if (line < 1)
      return;
   --line;
   //
   auto editor = this->ui.textEditor;
   auto doc    = editor->document();
   QTextBlock  block  = doc->findBlockByLineNumber(line);
   QTextCursor cursor = editor->textCursor();
   int pos = block.position();
   if (col > 0)
      pos += (col - 1);
   cursor.setPosition(pos);
   editor->setFocus();
   editor->setTextCursor(cursor);
}

void ScriptEditorPageScriptCode::updateCodeEditorStyle() {
   auto* widget = this->ui.textEditor;
   //
   QString qss;
   {
      const auto& setting = ReachINI::CodeEditor::sFontFamily;
      auto family = QString::fromUtf8(setting.currentStr.c_str());
      QFont font;
      font.setFamily(family);
      if (!font.exactMatch()) {
         family = QString::fromUtf8(setting.initialStr.c_str());
         font.setFamily(family);
      }
      font.setPointSize(10); // TODO: make configurable
      //
      qss += QString("font: %2pt \"%1\";").arg(font.family()).arg(font.pointSize());
   }
   if (ReachINI::CodeEditor::bOverrideBackColor.current.b) {
      cobb::qt::css_color_parse_error error;
      QString data = QString::fromUtf8(ReachINI::CodeEditor::sBackColor.currentStr.c_str());
      QColor  c    = cobb::qt::parse_css_color(data, error);
      if (error == cobb::qt::css_color_parse_error::none)
         qss += QString("background-color: %1;").arg(data);
   }
   if (ReachINI::CodeEditor::bOverrideTextColor.current.b) {
      cobb::qt::css_color_parse_error error;
      QString data = QString::fromUtf8(ReachINI::CodeEditor::sTextColor.currentStr.c_str());
      QColor  c    = cobb::qt::parse_css_color(data, error);
      if (error == cobb::qt::css_color_parse_error::none)
         qss += QString("color: %1;").arg(data);
   }
   qss = QString("QTextEdit { %1 }").arg(qss); // needed to prevent settings from bleeding into e.g. the scrollbar
   widget->setStyleSheet(qss);
}

QStringList ScriptEditorPageScriptCode::buildMegaloCompletionWords() {
   QStringList words = {
      // Keywords:
      QStringLiteral("alias"),
      QStringLiteral("alt"),
      QStringLiteral("altif"),
      QStringLiteral("and"),
      QStringLiteral("declare"),
      QStringLiteral("do"),
      QStringLiteral("else"),
      QStringLiteral("elseif"),
      QStringLiteral("end"),
      QStringLiteral("enum"),
      QStringLiteral("for"),
      QStringLiteral("function"),
      QStringLiteral("if"),
      QStringLiteral("inline"),
      QStringLiteral("not"),
      QStringLiteral("on"),
      QStringLiteral("or"),
      QStringLiteral("then"),

      // Keyword phrases and language words:
      QStringLiteral("each"),
      QStringLiteral("object"),
      QStringLiteral("player"),
      QStringLiteral("team"),
      QStringLiteral("randomly"),
      QStringLiteral("with"),
      QStringLiteral("label"),
      QStringLiteral("network"),
      QStringLiteral("priority"),
      QStringLiteral("low"),
      QStringLiteral("high"),
      QStringLiteral("local"),
      QStringLiteral("init"),
      QStringLiteral("pregame"),
      QStringLiteral("host"),
      QStringLiteral("migration"),
      QStringLiteral("double"),
      QStringLiteral("death"),

      // Core namespaces:
      QStringLiteral("global"),
      QStringLiteral("game"),
      QStringLiteral("enums"),
      QStringLiteral("temporaries"),

      // Frequently-used constants/built-ins:
      QStringLiteral("true"),
      QStringLiteral("false"),
      QStringLiteral("all_players"),
      QStringLiteral("current_object"),
      QStringLiteral("current_player"),
      QStringLiteral("current_team"),
      QStringLiteral("hud_player"),
      QStringLiteral("hud_player_team"),
      QStringLiteral("hud_target_object"),
      QStringLiteral("hud_target_player"),
      QStringLiteral("hud_target_team"),
      QStringLiteral("killed_object"),
      QStringLiteral("killer_object"),
      QStringLiteral("killer_player"),
      QStringLiteral("local_player"),
      QStringLiteral("local_team"),
      QStringLiteral("neutral_team"),
      QStringLiteral("no_object"),
      QStringLiteral("no_player"),
      QStringLiteral("no_team"),
      QStringLiteral("no_widget"),

      // Common enum stems:
      QStringLiteral("damage_reporting_modifier"),
      QStringLiteral("damage_reporting_type"),
      QStringLiteral("orientation"),

      // Frequently-used API names:
      QStringLiteral("rand"),
      QStringLiteral("send_incident"),
      QStringLiteral("apply_traits"),
      QStringLiteral("set_text"),
      QStringLiteral("set_visibility"),
      QStringLiteral("set_waypoint_icon"),
      QStringLiteral("set_waypoint_priority"),
      QStringLiteral("set_waypoint_visibility"),
   };
   words.removeDuplicates();
   words.sort(Qt::CaseInsensitive);
   return words;
}
void ScriptEditorPageScriptCode::setupAutocomplete() {
   this->_completionModel = new QStringListModel(buildMegaloCompletionWords(), this);
   this->_completer = new QCompleter(this->_completionModel, this);
   this->_completer->setCaseSensitivity(Qt::CaseInsensitive);
   this->_completer->setFilterMode(Qt::MatchStartsWith);
   this->_completer->setCompletionMode(QCompleter::PopupCompletion);
   this->_completer->setWrapAround(false);
   this->_completer->setWidget(this->ui.textEditor);

   QObject::connect(this->_completer, QOverload<const QString&>::of(&QCompleter::activated), this, [this](const QString& completion) {
      this->applyCompletion(completion);
   });
   QObject::connect(this->ui.textEditor, &QTextEdit::textChanged, this, [this]() {
      if (this->_isApplyingCompletion)
         return;
      this->showAutocompletePopup(false);
   });

   auto* shortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Space), this->ui.textEditor);
   QObject::connect(shortcut, &QShortcut::activated, this, [this]() {
      this->showAutocompletePopup(true);
   });

   this->ui.textEditor->installEventFilter(this);
   this->_completer->popup()->installEventFilter(this);
}
QString ScriptEditorPageScriptCode::completionPrefixUnderCursor() const {
   auto cursor = this->ui.textEditor->textCursor();
   cursor.select(QTextCursor::WordUnderCursor);
   return cursor.selectedText();
}
void ScriptEditorPageScriptCode::showAutocompletePopup(bool force) {
   if (!this->_completer || !this->ui.textEditor->hasFocus())
      return;

   QString prefix = this->completionPrefixUnderCursor();
   if (!force && prefix.size() < 2) {
      this->_completer->popup()->hide();
      return;
   }

   this->_completer->setCompletionPrefix(prefix);
   if (this->_completer->completionCount() <= 0) {
      this->_completer->popup()->hide();
      return;
   }

   auto* popup = this->_completer->popup();
   popup->setCurrentIndex(this->_completer->completionModel()->index(0, 0));

   QRect cr = this->ui.textEditor->cursorRect();
   int width = popup->sizeHint().width() + popup->verticalScrollBar()->sizeHint().width() + 8;
   if (width < 180)
      width = 180;
   cr.setWidth(width);
   this->_completer->complete(cr);
}
void ScriptEditorPageScriptCode::applyCompletion(const QString& completion) {
   if (completion.isEmpty())
      return;

   auto cursor = this->ui.textEditor->textCursor();
   cursor.select(QTextCursor::WordUnderCursor);
   this->_isApplyingCompletion = true;
   cursor.insertText(completion);
   this->_isApplyingCompletion = false;
   this->ui.textEditor->setTextCursor(cursor);
}
bool ScriptEditorPageScriptCode::eventFilter(QObject* watched, QEvent* event) {
   if (this->_completer && event->type() == QEvent::KeyPress) {
      bool from_editor = watched == this->ui.textEditor;
      bool from_popup  = watched == this->_completer->popup();
      if (!from_editor && !from_popup)
         return QWidget::eventFilter(watched, event);

      auto* key_event = static_cast<QKeyEvent*>(event);
      auto* popup = this->_completer->popup();
      if (popup && popup->isVisible()) {
         switch (key_event->key()) {
            case Qt::Key_Escape:
               popup->hide();
               return true;
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
            case Qt::Key_Return:
            case Qt::Key_Enter: {
               auto index = popup->currentIndex();
               if (index.isValid()) {
                  this->applyCompletion(index.data(Qt::DisplayRole).toString());
                  popup->hide();
                  return true;
               }
               break;
            }
            case Qt::Key_Down:
            case Qt::Key_Up:
            case Qt::Key_PageDown:
            case Qt::Key_PageUp:
               if (from_editor) {
                  QCoreApplication::sendEvent(popup, key_event);
                  return true;
               }
               break;
         }
      }
      if (from_editor && key_event->key() == Qt::Key_Period) {
         QTimer::singleShot(0, this, [this]() {
            this->showAutocompletePopup(true);
         });
      }
   }
   return QWidget::eventFilter(watched, event);
}
