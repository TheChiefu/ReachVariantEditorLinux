#pragma once
#include <QDialog>
#include "ui_reference_popup.h"

class ScriptEditorReferencePopup : public QDialog {
   Q_OBJECT
   public:
      explicit ScriptEditorReferencePopup(QWidget* parent = nullptr);

   protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

   private:
      void openHref(const QString& href);

   private:
      Ui::ScriptEditorReferencePopup ui;
};
