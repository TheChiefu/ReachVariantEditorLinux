#include "page_script_code.h"
#include "find_replace_bar.h"
#include "reference_popup.h"
#include <array>
#include <utility>
#include "compiler_unresolved_strings.h"
#include <QApplication>
#include <QCompleter>
#include <QKeyEvent>
#include <QMessageBox>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QScrollBar>
#include <QShowEvent>
#include <QShortcut>
#include <QSet>
#include <QSplitter>
#include <QTextDocument>
#include <QStringListModel>
#include <QStringView>
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
   struct ce_dynamic_symbols {
      QStringList words;
      QStringList enum_types;
      QHash<QString, QStringList> enum_values_by_type; // lower-case enum name -> values
   };

   const std::array ini_settings = {
      &ReachINI::CodeEditor::bOverrideBackColor,
      &ReachINI::CodeEditor::bOverrideTextColor,
      &ReachINI::CodeEditor::bOverrideLineNumberBackColor,
      &ReachINI::CodeEditor::bOverrideLineNumberTextColor,
      &ReachINI::CodeEditor::bOverrideLineNumberCurrentColor,
      &ReachINI::CodeEditor::sBackColor,
      &ReachINI::CodeEditor::sTextColor,
      &ReachINI::CodeEditor::sLineNumberBackColor,
      &ReachINI::CodeEditor::sLineNumberTextColor,
      &ReachINI::CodeEditor::sLineNumberCurrentColor,
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
   QString _leading_indentation(const QString& line) {
      int i = 0;
      for (; i < line.size(); ++i) {
         QChar c = line[i];
         if (c != ' ' && c != '\t')
            break;
      }
      return line.left(i);
   }
   void _append_unique_case_insensitive(QStringList& list, const QString& value) {
      if (value.isEmpty())
         return;
      if (!_contains_case_insensitive(list, value))
         list.push_back(value);
   }
   QStringList _merge_unique_sorted(const QStringList& a, const QStringList& b) {
      QStringList out = a;
      out.reserve(a.size() + b.size());
      for (const auto& value : b)
         _append_unique_case_insensitive(out, value);
      out.sort(Qt::CaseInsensitive);
      return out;
   }
   ce_dynamic_symbols _collect_dynamic_symbols(const QString& script) {
      static const QRegularExpression alias_re(
         QStringLiteral("^\\s*alias\\s+([A-Za-z_][A-Za-z0-9_]*)\\s*="),
         QRegularExpression::CaseInsensitiveOption
      );
      static const QRegularExpression enum_re(
         QStringLiteral("^\\s*enum\\s+([A-Za-z_][A-Za-z0-9_]*)\\b"),
         QRegularExpression::CaseInsensitiveOption
      );
      static const QRegularExpression function_re(
         QStringLiteral("^\\s*function\\s+([A-Za-z_][A-Za-z0-9_]*)\\s*\\("),
         QRegularExpression::CaseInsensitiveOption
      );
      static const QRegularExpression enum_value_re(
         QStringLiteral("^\\s*([A-Za-z_][A-Za-z0-9_]*)\\b"),
         QRegularExpression::CaseInsensitiveOption
      );
      static const QRegularExpression enum_end_re(
         QStringLiteral("^\\s*end\\b"),
         QRegularExpression::CaseInsensitiveOption
      );

      ce_dynamic_symbols out;
      QSet<QString> seen_words;
      QSet<QString> seen_enum_types;

      auto add_word = [&seen_words, &out](const QString& word) {
         if (word.isEmpty())
            return;
         QString key = word.toLower();
         if (seen_words.contains(key))
            return;
         seen_words.insert(key);
         out.words.push_back(word);
      };
      auto add_enum_type = [&seen_enum_types, &out](const QString& word) {
         if (word.isEmpty())
            return;
         QString key = word.toLower();
         if (seen_enum_types.contains(key))
            return;
         seen_enum_types.insert(key);
         out.enum_types.push_back(word);
      };
      auto add_enum_value = [&out](const QString& enum_key, const QString& value) {
         if (enum_key.isEmpty() || value.isEmpty())
            return;
         auto& values = out.enum_values_by_type[enum_key];
         _append_unique_case_insensitive(values, value);
      };

      bool in_enum = false;
      QString current_enum_key;
      const auto lines = QStringView(script).split('\n');
      for (QStringView line_view : lines) {
         QString line = line_view.toString();
         int comment = line.indexOf(QStringLiteral("--"));
         if (comment >= 0)
            line.truncate(comment);
         line = line.trimmed();
         if (line.isEmpty())
            continue;

         if (in_enum) {
            if (enum_end_re.match(line).hasMatch()) {
               in_enum = false;
               current_enum_key.clear();
               continue;
            }
            auto value_match = enum_value_re.match(line);
            if (value_match.hasMatch())
               add_enum_value(current_enum_key, value_match.captured(1));
            continue;
         }

         auto alias_match = alias_re.match(line);
         if (alias_match.hasMatch()) {
            add_word(alias_match.captured(1));
            continue;
         }
         auto enum_match = enum_re.match(line);
         if (enum_match.hasMatch()) {
            QString enum_name = enum_match.captured(1);
            add_word(enum_name);
            add_enum_type(enum_name);
            in_enum = true;
            current_enum_key = enum_name.toLower();
            continue;
         }
         auto function_match = function_re.match(line);
         if (function_match.hasMatch()) {
            add_word(function_match.captured(1));
            continue;
         }
      }

      out.words.sort(Qt::CaseInsensitive);
      out.enum_types.sort(Qt::CaseInsensitive);
      for (auto it = out.enum_values_by_type.begin(); it != out.enum_values_by_type.end(); ++it)
         it.value().sort(Qt::CaseInsensitive);
      return out;
   }
}

ScriptEditorPageScriptCode::ScriptEditorPageScriptCode(QWidget* parent) : QWidget(parent) {
   ui.setupUi(this);
   this->ui.splitter->setStretchFactor(0, 7);
   this->ui.splitter->setStretchFactor(1, 3);
   this->ui.referencePanel->hide();
   this->ui.referenceSplitter->setHandleWidth(0);
   {
      auto sizes = this->ui.referenceSplitter->sizes();
      if (sizes.size() >= 2) {
         sizes[0] += sizes[1];
         sizes[1] = 0;
         this->ui.referenceSplitter->setSizes(sizes);
      }
   }
   {
      auto sizes = this->ui.splitter->sizes();
      if (sizes.size() >= 2 && sizes[1] > 0)
         this->_compileLogExpandedSize = sizes[1];
      this->updateCompileLogCollapseButton();
   }
   {
      new MegaloSyntaxHighlighter(this->ui.textEditor->document());
      this->updateCodeEditorStyle();
      this->setupAutocomplete();
      this->setupFindReplaceBar();
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
      this->loadCodeText(decompiler.current_content, true);
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
      this->setCompileLogCollapsed(false);
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
   QObject::connect(this->ui.buttonCompileLogCollapse, &QPushButton::clicked, [this]() {
      this->setCompileLogCollapsed(!this->isCompileLogCollapsed());
   });
   QObject::connect(this->ui.splitter, &QSplitter::splitterMoved, [this](int, int) {
      auto sizes = this->ui.splitter->sizes();
      if (!this->isCompileLogCollapsed() && sizes.size() >= 2 && sizes[1] > 0)
         this->_compileLogExpandedSize = sizes[1];
      this->updateCompileLogCollapseButton();
   });
   QObject::connect(this->ui.buttonReferencePopup, &QPushButton::clicked, [this]() {
      this->showReferencePopup();
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
QString ScriptEditorPageScriptCode::currentCodeText() const {
   return this->ui.textEditor->toPlainText();
}
void ScriptEditorPageScriptCode::loadCodeText(const QString& text, bool mark_clean) {
   const QSignalBlocker blocker(this->ui.textEditor);
   this->ui.textEditor->setPlainText(text);
   this->rebuildDynamicAutocompleteSymbols();
   if (mark_clean)
      this->ui.textEditor->document()->setModified(false);
}
bool ScriptEditorPageScriptCode::hasUnsavedEditorTextChanges() const {
   return this->ui.textEditor->document()->isModified();
}
bool ScriptEditorPageScriptCode::isCompileLogCollapsed() const {
   auto sizes = this->ui.splitter->sizes();
   return sizes.size() >= 2 && sizes[1] <= 0;
}
void ScriptEditorPageScriptCode::setCompileLogCollapseButtonState(bool collapsed) {
   this->ui.buttonCompileLogCollapse->setText(collapsed ? QStringLiteral("⇧") : QStringLiteral("⇩"));
   this->ui.buttonCompileLogCollapse->setToolTip(collapsed ? tr("Show compiler log") : tr("Hide compiler log"));
}
void ScriptEditorPageScriptCode::setCompileLogCollapsed(bool collapsed) {
   auto sizes = this->ui.splitter->sizes();
   if (sizes.size() < 2) {
      this->setCompileLogCollapseButtonState(collapsed);
      return;
   }
   int total = sizes[0] + sizes[1];
   if (total <= 0) {
      this->setCompileLogCollapseButtonState(collapsed);
      return;
   }
   if (collapsed) {
      if (sizes[1] > 0)
         this->_compileLogExpandedSize = sizes[1];
      sizes[0] = total;
      sizes[1] = 0;
   } else {
      int log_size = this->_compileLogExpandedSize;
      if (log_size <= 0)
         log_size = total / 3;
      if (log_size < 96)
         log_size = 96;
      if (log_size >= total)
         log_size = total - 1;
      if (log_size < 0)
         log_size = 0;
      sizes[1] = log_size;
      sizes[0] = total - log_size;
   }
   this->ui.splitter->setSizes(sizes);
   this->setCompileLogCollapseButtonState(collapsed);
   QTimer::singleShot(0, this, [this]() {
      this->updateCompileLogCollapseButton();
   });
}
void ScriptEditorPageScriptCode::updateCompileLogCollapseButton() {
   this->setCompileLogCollapseButtonState(this->isCompileLogCollapsed());
}
void ScriptEditorPageScriptCode::showReferencePopup() {
   if (!this->_referencePopup) {
      this->_referencePopup = new ScriptEditorReferencePopup(this);
   }
   this->_referencePopup->show();
   this->_referencePopup->raise();
   this->_referencePopup->activateWindow();
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
   auto parse_color_setting = [](const cobb::ini::setting& setting) -> QColor {
      cobb::qt::css_color_parse_error error;
      auto value = QString::fromUtf8(setting.currentStr.c_str());
      auto color = cobb::qt::parse_css_color(value, error);
      if (error == cobb::qt::css_color_parse_error::none)
         return color;
      error = cobb::qt::css_color_parse_error::none;
      value = QString::fromUtf8(setting.initialStr.c_str());
      color = cobb::qt::parse_css_color(value, error);
      if (error == cobb::qt::css_color_parse_error::none)
         return color;
      return QColor();
   };

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
   qss = QString("QPlainTextEdit { %1 }").arg(qss); // needed to prevent settings from bleeding into e.g. the scrollbar
   widget->setStyleSheet(qss);

   QColor gutter_background;
   QColor gutter_text;
   QColor gutter_current_text;
   if (ReachINI::CodeEditor::bOverrideLineNumberBackColor.current.b)
      gutter_background = parse_color_setting(ReachINI::CodeEditor::sLineNumberBackColor);
   if (ReachINI::CodeEditor::bOverrideLineNumberTextColor.current.b)
      gutter_text = parse_color_setting(ReachINI::CodeEditor::sLineNumberTextColor);
   if (ReachINI::CodeEditor::bOverrideLineNumberCurrentColor.current.b)
      gutter_current_text = parse_color_setting(ReachINI::CodeEditor::sLineNumberCurrentColor);
   widget->setLineNumberAreaColors(gutter_background, gutter_text, gutter_current_text);
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
void ScriptEditorPageScriptCode::rebuildDynamicAutocompleteSymbols() {
   auto symbols = _collect_dynamic_symbols(this->ui.textEditor->toPlainText());
   this->_dynamicCompletionWords = std::move(symbols.words);
   this->_dynamicEnumTypes = std::move(symbols.enum_types);
   this->_dynamicEnumValuesByType = std::move(symbols.enum_values_by_type);
   this->_defaultCompletionWords = _merge_unique_sorted(this->_baseCompletionWords, this->_dynamicCompletionWords);
}
QStringList ScriptEditorPageScriptCode::dynamicEnumValuesForType(const QString& type) const {
   if (type.isEmpty())
      return {};
   auto it = this->_dynamicEnumValuesByType.constFind(type.toLower());
   if (it == this->_dynamicEnumValuesByType.cend())
      return {};
   return it.value();
}
int ScriptEditorPageScriptCode::countMatches(const QString& find_text) const {
   if (find_text.isEmpty())
      return 0;
   int count = 0;
   auto* doc = this->ui.textEditor->document();
   QTextCursor match = doc->find(find_text, 0);
   while (!match.isNull()) {
      ++count;
      match = doc->find(find_text, match.position());
   }
   return count;
}
void ScriptEditorPageScriptCode::updateFindReplaceMatchCount() {
   if (!this->_findReplaceBar)
      return;
   QString text = this->_findReplaceBar->findText();
   if (text.isEmpty()) {
      this->_findReplaceBar->setMatchCount(-1);
      return;
   }
   this->_findReplaceBar->setMatchCount(this->countMatches(text));
}
bool ScriptEditorPageScriptCode::findTextWithWrap(const QString& find_text) {
   if (find_text.isEmpty())
      return false;
   if (this->ui.textEditor->find(find_text))
      return true;
   auto cursor = this->ui.textEditor->textCursor();
   cursor.movePosition(QTextCursor::Start);
   this->ui.textEditor->setTextCursor(cursor);
   return this->ui.textEditor->find(find_text);
}
bool ScriptEditorPageScriptCode::findPreviousWithWrap(const QString& find_text) {
   if (find_text.isEmpty())
      return false;
   if (this->ui.textEditor->find(find_text, QTextDocument::FindBackward))
      return true;
   auto cursor = this->ui.textEditor->textCursor();
   cursor.movePosition(QTextCursor::End);
   this->ui.textEditor->setTextCursor(cursor);
   return this->ui.textEditor->find(find_text, QTextDocument::FindBackward);
}
bool ScriptEditorPageScriptCode::replaceNext(const QString& find_text, const QString& replace_text) {
   if (find_text.isEmpty())
      return false;

   auto cursor = this->ui.textEditor->textCursor();
   if (cursor.hasSelection() && cursor.selectedText() == find_text) {
      this->_isApplyingCompletion = true;
      cursor.insertText(replace_text);
      this->_isApplyingCompletion = false;
      this->ui.textEditor->setTextCursor(cursor);
      this->rebuildDynamicAutocompleteSymbols();
      return true;
   }
   if (!this->findTextWithWrap(find_text))
      return false;

   cursor = this->ui.textEditor->textCursor();
   if (!cursor.hasSelection())
      return false;
   this->_isApplyingCompletion = true;
   cursor.insertText(replace_text);
   this->_isApplyingCompletion = false;
   this->ui.textEditor->setTextCursor(cursor);
   this->rebuildDynamicAutocompleteSymbols();
   return true;
}
int ScriptEditorPageScriptCode::replaceAll(const QString& find_text, const QString& replace_text) {
   if (find_text.isEmpty())
      return 0;

   int count = 0;
   auto* doc = this->ui.textEditor->document();
   QTextCursor edit_cursor(doc);
   this->_isApplyingCompletion = true;
   edit_cursor.beginEditBlock();

   QTextCursor match = doc->find(find_text, 0);
   while (!match.isNull()) {
      match.insertText(replace_text);
      ++count;
      match = doc->find(find_text, match.position());
   }

   edit_cursor.endEditBlock();
   this->_isApplyingCompletion = false;
   this->ui.textEditor->setTextCursor(edit_cursor);
   this->rebuildDynamicAutocompleteSymbols();
   return count;
}
void ScriptEditorPageScriptCode::setupFindReplaceBar() {
   if (this->_findReplaceBar)
      return;
   this->_findReplaceBar = new ScriptEditorFindReplaceBar(this->ui.textEditor);
   this->_findReplaceBar->hide();
   QObject::connect(this->_findReplaceBar, &ScriptEditorFindReplaceBar::findTextChanged, this, [this](const QString& text) {
      this->_lastFindText = text;
      this->updateFindReplaceMatchCount();
      this->_findReplaceBar->clearStatus();
   });
   QObject::connect(this->_findReplaceBar, &ScriptEditorFindReplaceBar::requestClose, this, [this]() {
      this->_findReplaceBar->hide();
      this->ui.textEditor->setFocus();
   });
   QObject::connect(this->_findReplaceBar, &ScriptEditorFindReplaceBar::requestFindNext, this, [this](const QString& find_text) {
      this->_lastFindText = find_text;
      if (find_text.isEmpty()) {
         this->_findReplaceBar->setStatus(tr("Type text to find."), true);
         return;
      }
      bool found = this->findTextWithWrap(find_text);
      if (!found)
         this->_findReplaceBar->setStatus(tr("No results."), true);
      else
         this->_findReplaceBar->clearStatus();
      this->updateFindReplaceMatchCount();
      QTimer::singleShot(0, this, [this]() {
         if (this->_findReplaceBar && this->_findReplaceBar->isVisible())
            this->_findReplaceBar->focusFindField();
      });
   });
   QObject::connect(this->_findReplaceBar, &ScriptEditorFindReplaceBar::requestFindPrevious, this, [this](const QString& find_text) {
      this->_lastFindText = find_text;
      if (find_text.isEmpty()) {
         this->_findReplaceBar->setStatus(tr("Type text to find."), true);
         return;
      }
      bool found = this->findPreviousWithWrap(find_text);
      if (!found)
         this->_findReplaceBar->setStatus(tr("No results."), true);
      else
         this->_findReplaceBar->clearStatus();
      this->updateFindReplaceMatchCount();
      QTimer::singleShot(0, this, [this]() {
         if (this->_findReplaceBar && this->_findReplaceBar->isVisible())
            this->_findReplaceBar->focusFindField();
      });
   });
   QObject::connect(this->_findReplaceBar, &ScriptEditorFindReplaceBar::requestReplaceNext, this, [this](const QString& find_text, const QString& replace_text) {
      this->_lastFindText = find_text;
      this->_lastReplaceText = replace_text;
      if (find_text.isEmpty()) {
         this->_findReplaceBar->setStatus(tr("Type text to find."), true);
         return;
      }
      bool replaced = this->replaceNext(find_text, replace_text);
      if (!replaced)
         this->_findReplaceBar->setStatus(tr("No results."), true);
      else
         this->_findReplaceBar->setStatus(tr("Replaced one match."));
      this->updateFindReplaceMatchCount();
      QTimer::singleShot(0, this, [this]() {
         if (this->_findReplaceBar && this->_findReplaceBar->isVisible())
            this->_findReplaceBar->focusFindField();
      });
   });
   QObject::connect(this->_findReplaceBar, &ScriptEditorFindReplaceBar::requestReplaceAll, this, [this](const QString& find_text, const QString& replace_text) {
      this->_lastFindText = find_text;
      this->_lastReplaceText = replace_text;
      if (find_text.isEmpty()) {
         this->_findReplaceBar->setStatus(tr("Type text to find."), true);
         return;
      }
      int replaced = this->replaceAll(find_text, replace_text);
      if (replaced == 0)
         this->_findReplaceBar->setStatus(tr("No results."), true);
      else
         this->_findReplaceBar->setStatus(tr("Replaced %1 occurrence(s).").arg(replaced));
      this->updateFindReplaceMatchCount();
      QTimer::singleShot(0, this, [this]() {
         if (this->_findReplaceBar && this->_findReplaceBar->isVisible())
            this->_findReplaceBar->focusFindField();
      });
   });
   this->ui.textEditor->viewport()->installEventFilter(this);
   QObject::connect(this->ui.textEditor->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int) {
      this->repositionFindReplaceBar();
   });
   QObject::connect(this->ui.textEditor->horizontalScrollBar(), &QScrollBar::valueChanged, this, [this](int) {
      this->repositionFindReplaceBar();
   });
   this->repositionFindReplaceBar();
}
void ScriptEditorPageScriptCode::showFindReplaceBar(bool show_replace) {
   this->setupFindReplaceBar();
   QString selected = this->ui.textEditor->textCursor().selectedText();
   selected.replace(QChar::ParagraphSeparator, QChar(' '));
   if (!selected.isEmpty())
      this->_lastFindText = selected;
   QString find_text = selected.isEmpty() ? this->_lastFindText : selected;
   if (show_replace)
      this->_findReplaceBar->showReplace(find_text, this->_lastReplaceText);
   else
      this->_findReplaceBar->showFind(find_text);
   this->updateFindReplaceMatchCount();
   this->repositionFindReplaceBar();
}
void ScriptEditorPageScriptCode::repositionFindReplaceBar() {
   if (!this->_findReplaceBar)
      return;
   this->_findReplaceBar->adjustSize();
   QRect viewport_rect = this->ui.textEditor->viewport()->geometry();
   const int margin = 6;
   int x = viewport_rect.right() - this->_findReplaceBar->width() - margin + 1;
   if (x < margin)
      x = margin;
   int y = viewport_rect.top() + margin;
   this->_findReplaceBar->move(x, y);
   this->_findReplaceBar->raise();
}
void ScriptEditorPageScriptCode::setupAutocomplete() {
   this->_baseCompletionWords = buildMegaloCompletionWords();
   this->rebuildDynamicAutocompleteSymbols();
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
   QObject::connect(this->ui.textEditor, &QPlainTextEdit::textChanged, this, [this]() {
      if (this->_isApplyingCompletion)
         return;
      this->rebuildDynamicAutocompleteSymbols();
      this->showAutocompletePopup(false);
   });

   auto* shortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Space), this->ui.textEditor);
   QObject::connect(shortcut, &QShortcut::activated, this, [this]() {
      this->showAutocompletePopup(true);
   });
   auto* find_shortcut = new QShortcut(QKeySequence::Find, this->ui.textEditor);
   QObject::connect(find_shortcut, &QShortcut::activated, this, [this]() { this->showFindReplaceBar(false); });
   auto* replace_shortcut = new QShortcut(QKeySequence::Replace, this->ui.textEditor);
   QObject::connect(replace_shortcut, &QShortcut::activated, this, [this]() { this->showFindReplaceBar(true); });
   auto* replace_ctrl_h_shortcut = new QShortcut(QKeySequence(QStringLiteral("Ctrl+H")), this->ui.textEditor);
   QObject::connect(replace_ctrl_h_shortcut, &QShortcut::activated, this, [this]() { this->showFindReplaceBar(true); });

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

   auto builtin_enum_values_for_type = [](const QString& type) -> QStringList {
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
         return _merge_unique_sorted(ce_context_enum_types, this->_dynamicEnumTypes);
      return _merge_unique_sorted(
         builtin_enum_values_for_type(segments[1]),
         this->dynamicEnumValuesForType(segments[1])
      );
   }
   if (segments.size() == 1) {
      QStringList builtins = builtin_enum_values_for_type(first);
      QStringList dynamic  = this->dynamicEnumValuesForType(first);
      if (!builtins.isEmpty() || !dynamic.isEmpty())
         return _merge_unique_sorted(builtins, dynamic);
   }

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
void ScriptEditorPageScriptCode::showEvent(QShowEvent* event) {
   QWidget::showEvent(event);
   QTimer::singleShot(0, this, [this]() {
      if (!this->_compileLogInitialized) {
         this->_compileLogInitialized = true;
         this->setCompileLogCollapsed(true);
      } else {
         auto sizes = this->ui.splitter->sizes();
         if (sizes.size() >= 2 && sizes[1] > 0)
            this->_compileLogExpandedSize = sizes[1];
         this->updateCompileLogCollapseButton();
      }
   });
}
bool ScriptEditorPageScriptCode::eventFilter(QObject* watched, QEvent* event) {
   if ((watched == this->ui.textEditor || watched == this->ui.textEditor->viewport()) && event->type() == QEvent::Resize)
      this->repositionFindReplaceBar();

   if (this->_completer && event->type() == QEvent::KeyPress) {
      bool from_editor = watched == this->ui.textEditor;
      bool from_popup  = watched == this->_completer->popup();
      if (!from_editor && !from_popup)
         return QWidget::eventFilter(watched, event);

      auto* key_event = static_cast<QKeyEvent*>(event);
      auto* popup = this->_completer->popup();
      if (from_editor && key_event->key() == Qt::Key_Escape && this->_findReplaceBar && this->_findReplaceBar->isVisible()) {
         this->_findReplaceBar->hide();
         return true;
      }
      if (from_editor && (key_event->key() == Qt::Key_Return || key_event->key() == Qt::Key_Enter) && this->_findReplaceBar && this->_findReplaceBar->isVisible()) {
         auto* focused = QApplication::focusWidget();
         if (focused && this->_findReplaceBar->isAncestorOf(focused))
            return true;
      }
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
      if (from_editor && (key_event->key() == Qt::Key_Return || key_event->key() == Qt::Key_Enter)) {
         Qt::KeyboardModifiers mods = key_event->modifiers();
         if (!(mods & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))) {
            auto cursor = this->ui.textEditor->textCursor();
            QString indent = _leading_indentation(cursor.block().text());
            this->_isApplyingCompletion = true;
            cursor.beginEditBlock();
            cursor.insertText(QStringLiteral("\n") + indent);
            cursor.endEditBlock();
            this->_isApplyingCompletion = false;
            this->ui.textEditor->setTextCursor(cursor);
            return true;
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
