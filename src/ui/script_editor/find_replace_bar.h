#pragma once
#include "ui_find_replace_bar.h"
#include <QFrame>

class ScriptEditorFindReplaceBar : public QFrame {
   Q_OBJECT
   public:
      explicit ScriptEditorFindReplaceBar(QWidget* parent = nullptr);

      void showFind(const QString& initial_find = QString());
      void showReplace(const QString& initial_find = QString(), const QString& initial_replace = QString());
      QString findText() const;
      QString replaceText() const;
      void focusFindField();
      void setMatchCount(int);
      void setStatus(const QString&, bool error = false);
      void clearStatus();

   signals:
      void findTextChanged(const QString&);
      void requestFindNext(const QString&);
      void requestFindPrevious(const QString&);
      void requestReplaceNext(const QString&, const QString&);
      void requestReplaceAll(const QString&, const QString&);
      void requestClose();

   private:
      Ui::ScriptEditorFindReplaceBar ui;
};
