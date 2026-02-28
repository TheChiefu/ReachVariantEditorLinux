#pragma once
#include "ui_page_script_code.h"
#include <QHash>
#include <QStringList>
#include "../../editor_state.h"
#include "../../game_variants/components/megalo/compiler/compiler.h"
#include "../../game_variants/components/megalo/decompiler/decompiler.h"

class QCompleter;
class QEvent;
class QStringListModel;
class ScriptEditorFindReplaceBar;

class ScriptEditorPageScriptCode : public QWidget {
   Q_OBJECT
   public:
      using Compiler = Megalo::Compiler;
      using log_t    = Compiler::log_t;
      using pos_t    = Compiler::pos;
      //
   public:
      ScriptEditorPageScriptCode(QWidget* parent = nullptr);
      //
   protected:
      Ui::ScriptEditorPageScriptCode ui;
      //
      log_t _lastNotices;
      log_t _lastWarnings;
      log_t _lastErrors;
      log_t _lastFatals;
      bool _isApplyingCompletion = false;
      QCompleter* _completer = nullptr;
      QStringListModel* _completionModel = nullptr;
      QStringList _defaultCompletionWords;
      ScriptEditorFindReplaceBar* _findReplaceBar = nullptr;
      //
      void updateLog(Compiler&);
      void redrawLog();

      void jumpToLogItem(QListWidgetItem&);

      void updateCodeEditorStyle();
      void setupAutocomplete();
      void setupFindReplaceBar();
      void showFindReplaceBar(bool show_replace);
      void repositionFindReplaceBar();
      void rebuildDynamicAutocompleteSymbols();
      void showAutocompletePopup(bool force = false);
      QString completionPrefixUnderCursor() const;
      QString contextExpressionBeforeCursor() const;
      QStringList completionWordsForContextExpression(const QString&) const;
      QStringList dynamicEnumValuesForType(const QString&) const;
      int countMatches(const QString&) const;
      void updateFindReplaceMatchCount();
      bool findTextWithWrap(const QString&);
      bool findPreviousWithWrap(const QString&);
      bool replaceNext(const QString&, const QString&);
      int replaceAll(const QString&, const QString&);
      void applyCompletion(const QString&);
      static QStringList buildMegaloCompletionWords();

      bool eventFilter(QObject* watched, QEvent* event) override;

      QStringList _baseCompletionWords;
      QStringList _dynamicCompletionWords;
      QStringList _dynamicEnumTypes;
      QHash<QString, QStringList> _dynamicEnumValuesByType;
      QString _lastFindText;
      QString _lastReplaceText;
};
