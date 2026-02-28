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

   const QStringList ce_context_global_types = {
      QStringLiteral("number"),
      QStringLiteral("object"),
      QStringLiteral("player"),
      QStringLiteral("team"),
      QStringLiteral("timer"),
   };
   const QStringList ce_context_game_members = {
      QStringLiteral("betrayal_booting"),
      QStringLiteral("betrayal_penalty"),
      QStringLiteral("current_round"),
      QStringLiteral("friendly_fire"),
      QStringLiteral("grace_period"),
      QStringLiteral("grenades_on_map"),
      QStringLiteral("indestructible_vehicles"),
      QStringLiteral("lives_per_round"),
      QStringLiteral("loadout_cam_time"),
      QStringLiteral("perfection_enabled"),
      QStringLiteral("powerup_duration_red"),
      QStringLiteral("powerup_duration_blue"),
      QStringLiteral("powerup_duration_yellow"),
      QStringLiteral("respawn_growth"),
      QStringLiteral("respawn_time"),
      QStringLiteral("respawn_traits_duration"),
      QStringLiteral("round_limit"),
      QStringLiteral("round_time_limit"),
      QStringLiteral("rounds_to_win"),
      QStringLiteral("score_to_win"),
      QStringLiteral("proximity_voice"),
      QStringLiteral("dont_team_restrict_chat"),
      QStringLiteral("dead_players_can_talk"),
      QStringLiteral("sudden_death_time"),
      QStringLiteral("suicide_penalty"),
      QStringLiteral("symmetry"),
      QStringLiteral("symmetry_getter"),
      QStringLiteral("team_lives_per_round"),
      QStringLiteral("teams_enabled"),
      QStringLiteral("fireteams_enabled"),
      QStringLiteral("round_timer"),
      QStringLiteral("sudden_death_timer"),
      QStringLiteral("grace_period_timer"),
   };
   const QStringList ce_context_enum_types = {
      QStringLiteral("damage_reporting_modifier"),
      QStringLiteral("damage_reporting_type"),
      QStringLiteral("orientation"),
   };
   const QStringList ce_context_enum_values_damage_reporting_modifier = {
      QStringLiteral("none"),
      QStringLiteral("pummel"),
      QStringLiteral("assassination"),
      QStringLiteral("splatter"),
      QStringLiteral("sticky"),
      QStringLiteral("headshot"),
   };
   const QStringList ce_context_enum_values_orientation = {
      QStringLiteral("up_is_up"),
      QStringLiteral("upright"),
      QStringLiteral("right_is_up"),
      QStringLiteral("backward_is_up"),
      QStringLiteral("nose_down"),
      QStringLiteral("forward_is_up"),
      QStringLiteral("nose_up"),
      QStringLiteral("left_is_up"),
      QStringLiteral("down_is_up"),
      QStringLiteral("upside_down"),
   };
   const QStringList ce_context_enum_values_damage_reporting_type = {
      QStringLiteral("unknown"),
      QStringLiteral("guardians"),
      QStringLiteral("script"),
      QStringLiteral("ai_suicide"),
      QStringLiteral("magnum"),
      QStringLiteral("assault_rifle"),
      QStringLiteral("dmr"),
      QStringLiteral("shotgun"),
      QStringLiteral("sniper_rifle"),
      QStringLiteral("rocket_launcher"),
      QStringLiteral("spartan_laser"),
      QStringLiteral("frag_grenade"),
      QStringLiteral("grenade_launcher"),
      QStringLiteral("plasma_pistol"),
      QStringLiteral("needler"),
      QStringLiteral("plasma_rifle"),
      QStringLiteral("needle_rifle"),
      QStringLiteral("gravity_hammer"),
      QStringLiteral("energy_sword"),
      QStringLiteral("plasma_grenade"),
      QStringLiteral("concussion_rifle"),
      QStringLiteral("ghost"),
      QStringLiteral("warthog"),
      QStringLiteral("scorpion"),
      QStringLiteral("falcon"),
      QStringLiteral("fall_damage"),
      QStringLiteral("collision_damage"),
      QStringLiteral("melee_generic"),
      QStringLiteral("explosion_generic"),
      QStringLiteral("teleporter"),
      QStringLiteral("armor_lock_crush"),
      QStringLiteral("target_locator"),
      QStringLiteral("focus_rifle"),
      QStringLiteral("fuel_rod_gun"),
      QStringLiteral("sentinel_beam"),
   };
   const QStringList ce_context_player_members = {
      QStringLiteral("apply_traits"),
      QStringLiteral("biped"),
      QStringLiteral("score"),
      QStringLiteral("team"),
   };
   const QStringList ce_context_object_members = {
      QStringLiteral("set_waypoint_icon"),
      QStringLiteral("set_waypoint_priority"),
      QStringLiteral("set_waypoint_visibility"),
      QStringLiteral("team"),
   };
   const QStringList ce_context_team_members = {
      QStringLiteral("score"),
   };
   const QStringList ce_context_widget_members = {
      QStringLiteral("set_text"),
      QStringLiteral("set_visibility"),
   };
   const QStringList ce_player_like_symbols = {
      QStringLiteral("all_players"),
      QStringLiteral("current_player"),
      QStringLiteral("hud_player"),
      QStringLiteral("hud_target_player"),
      QStringLiteral("killer_player"),
      QStringLiteral("local_player"),
      QStringLiteral("no_player"),
      QStringLiteral("player"),
   };
   const QStringList ce_object_like_symbols = {
      QStringLiteral("biped"),
      QStringLiteral("current_object"),
      QStringLiteral("hud_target_object"),
      QStringLiteral("killed_object"),
      QStringLiteral("killer_object"),
      QStringLiteral("no_object"),
      QStringLiteral("object"),
   };
   const QStringList ce_team_like_symbols = {
      QStringLiteral("current_team"),
      QStringLiteral("hud_player_team"),
      QStringLiteral("hud_target_team"),
      QStringLiteral("local_team"),
      QStringLiteral("neutral_team"),
      QStringLiteral("no_team"),
      QStringLiteral("team"),
   };
   const QStringList ce_widget_like_symbols = {
      QStringLiteral("script_widget"),
   };
   const QStringList ce_timer_like_symbols = {
      QStringLiteral("round_timer"),
      QStringLiteral("sudden_death_timer"),
      QStringLiteral("grace_period_timer"),
      QStringLiteral("timer"),
   };

   bool _is_ident_char(QChar c) {
      return c.isLetterOrNumber() || c == '_';
   }
   bool _contains_case_insensitive(const QStringList& list, const QString& value) {
      for (const auto& item : list) {
         if (item.compare(value, Qt::CaseInsensitive) == 0)
            return true;
      }
      return false;
   }
   QStringList _extract_expression_segments(const QString& expression) {
      QStringList out;
      QString current;
      int bracket_depth = 0;
      for (QChar c : expression) {
         if (c == '[') {
            ++bracket_depth;
            continue;
         }
         if (c == ']') {
            if (bracket_depth > 0)
               --bracket_depth;
            continue;
         }
         if (bracket_depth > 0)
            continue;
         if (c == '.') {
            if (!current.isEmpty()) {
               out.push_back(current);
               current.clear();
            }
            continue;
         }
         if (_is_ident_char(c)) {
            current += c;
            continue;
         }
         if (!current.isEmpty()) {
            out.push_back(current);
            current.clear();
         }
      }
      if (!current.isEmpty())
         out.push_back(current);
      return out;
   }
   QStringList _filter_prefix(const QStringList& values, const QString& prefix) {
      if (prefix.isEmpty())
         return values;
      QStringList out;
      out.reserve(values.size());
      for (const auto& value : values) {
         if (value.startsWith(prefix, Qt::CaseInsensitive))
            out.push_back(value);
      }
      return out;
   }
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
   this->_defaultCompletionWords = buildMegaloCompletionWords();
   this->_completionModel = new QStringListModel(this->_defaultCompletionWords, this);
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
QString ScriptEditorPageScriptCode::contextExpressionBeforeCursor() const {
   auto cursor = this->ui.textEditor->textCursor();
   auto block  = cursor.block();
   int  pos_in_block = cursor.position() - block.position();
   if (pos_in_block <= 0)
      return QString();
   QString left = block.text().left(pos_in_block);
   int i = left.size() - 1;
   while (i >= 0 && left[i].isSpace())
      --i;
   while (i >= 0 && _is_ident_char(left[i]))
      --i;
   if (i < 0 || left[i] != '.')
      return QString();
   int end = i - 1;
   if (end < 0)
      return QString();
   int start = end;
   int bracket_depth = 0;
   for (; start >= 0; --start) {
      QChar c = left[start];
      if (c == ']') {
         ++bracket_depth;
         continue;
      }
      if (c == '[') {
         if (bracket_depth > 0) {
            --bracket_depth;
            continue;
         }
         break;
      }
      if (bracket_depth > 0)
         continue;
      if (_is_ident_char(c) || c == '.')
         continue;
      break;
   }
   ++start;
   if (start > end)
      return QString();
   return left.mid(start, end - start + 1);
}
QStringList ScriptEditorPageScriptCode::completionWordsForContextExpression(const QString& expression) const {
   if (expression.isEmpty())
      return {};
   QStringList segments = _extract_expression_segments(expression);
   if (segments.isEmpty())
      return {};

   auto enum_values_for_type = [](const QString& type) -> QStringList {
      if (type.compare(QStringLiteral("damage_reporting_modifier"), Qt::CaseInsensitive) == 0)
         return ce_context_enum_values_damage_reporting_modifier;
      if (type.compare(QStringLiteral("damage_reporting_type"), Qt::CaseInsensitive) == 0)
         return ce_context_enum_values_damage_reporting_type;
      if (type.compare(QStringLiteral("orientation"), Qt::CaseInsensitive) == 0)
         return ce_context_enum_values_orientation;
      return {};
   };

   const QString& first = segments.first();
   const QString& last  = segments.last();

   if (first.compare(QStringLiteral("enums"), Qt::CaseInsensitive) == 0) {
      if (segments.size() == 1)
         return ce_context_enum_types;
      return enum_values_for_type(segments[1]);
   }
   if (_contains_case_insensitive(ce_context_enum_types, first) && segments.size() == 1)
      return enum_values_for_type(first);

   if (first.compare(QStringLiteral("global"), Qt::CaseInsensitive) == 0 || first.compare(QStringLiteral("temporaries"), Qt::CaseInsensitive) == 0) {
      if (segments.size() == 1)
         return ce_context_global_types;
      if (last.compare(QStringLiteral("biped"), Qt::CaseInsensitive) == 0)
         return ce_context_object_members;
      if (last.compare(QStringLiteral("team"), Qt::CaseInsensitive) == 0)
         return ce_context_team_members;
      if (segments[1].compare(QStringLiteral("player"), Qt::CaseInsensitive) == 0)
         return ce_context_player_members;
      if (segments[1].compare(QStringLiteral("object"), Qt::CaseInsensitive) == 0)
         return ce_context_object_members;
      if (segments[1].compare(QStringLiteral("team"), Qt::CaseInsensitive) == 0)
         return ce_context_team_members;
      if (segments[1].compare(QStringLiteral("script_widget"), Qt::CaseInsensitive) == 0)
         return ce_context_widget_members;
   }
   if (first.compare(QStringLiteral("game"), Qt::CaseInsensitive) == 0) {
      if (segments.size() == 1)
         return ce_context_game_members;
      if (_contains_case_insensitive(ce_timer_like_symbols, last))
         return {};
   }

   if (_contains_case_insensitive(ce_widget_like_symbols, first) || _contains_case_insensitive(ce_widget_like_symbols, last))
      return ce_context_widget_members;
   if (_contains_case_insensitive(ce_player_like_symbols, last))
      return ce_context_player_members;
   if (_contains_case_insensitive(ce_object_like_symbols, last))
      return ce_context_object_members;
   if (_contains_case_insensitive(ce_team_like_symbols, last))
      return ce_context_team_members;
   if (_contains_case_insensitive(ce_timer_like_symbols, last))
      return {};

   return {};
}
void ScriptEditorPageScriptCode::showAutocompletePopup(bool force) {
   if (!this->_completer || !this->ui.textEditor->hasFocus())
      return;

   QString prefix = this->completionPrefixUnderCursor();
   QString context_expression = this->contextExpressionBeforeCursor();
   QStringList context_words = this->completionWordsForContextExpression(context_expression);
   bool has_context_words = !context_words.isEmpty();

   if (!force && !has_context_words && prefix.size() < 2) {
      this->_completer->popup()->hide();
      return;
   }

   if (has_context_words) {
      if (this->_completionModel->stringList() != context_words)
         this->_completionModel->setStringList(context_words);
   } else {
      if (this->_completionModel->stringList() != this->_defaultCompletionWords)
         this->_completionModel->setStringList(this->_defaultCompletionWords);
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
