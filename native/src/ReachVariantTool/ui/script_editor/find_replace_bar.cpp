#include "find_replace_bar.h"
#include <QShortcut>

ScriptEditorFindReplaceBar::ScriptEditorFindReplaceBar(QWidget* parent) : QFrame(parent) {
   this->ui.setupUi(this);
   this->setObjectName(QStringLiteral("scriptEditorFindReplaceBar"));

   this->ui.matchCount->setContentsMargins(0, 0, 0, 0);
   this->ui.statusLabel->setWordWrap(false);
   {
      int h = this->ui.statusLabel->sizeHint().height();
      if (h < 20)
         h = 20;
      this->ui.statusLabel->setMinimumHeight(h);
      this->ui.statusLabel->setMaximumHeight(h);
      this->ui.statusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      this->ui.statusLabel->setVisible(true);
   }
   this->setMatchCount(-1);
   this->ui.replaceRow->setVisible(false);
   this->ui.buttonToggleReplace->setChecked(false);
   this->clearStatus();

   QObject::connect(this->ui.findEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
      emit this->findTextChanged(text);
   });
   QObject::connect(this->ui.buttonNext, &QPushButton::clicked, this, [this]() {
      emit this->requestFindNext(this->ui.findEdit->text());
   });
   QObject::connect(this->ui.buttonPrev, &QPushButton::clicked, this, [this]() {
      emit this->requestFindPrevious(this->ui.findEdit->text());
   });
   QObject::connect(this->ui.findEdit, &QLineEdit::returnPressed, this, [this]() {
      emit this->requestFindNext(this->ui.findEdit->text());
   });
   QObject::connect(this->ui.buttonReplaceOne, &QPushButton::clicked, this, [this]() {
      emit this->requestReplaceNext(this->ui.findEdit->text(), this->ui.replaceEdit->text());
   });
   QObject::connect(this->ui.buttonReplaceAll, &QPushButton::clicked, this, [this]() {
      emit this->requestReplaceAll(this->ui.findEdit->text(), this->ui.replaceEdit->text());
   });
   QObject::connect(this->ui.replaceEdit, &QLineEdit::returnPressed, this, [this]() {
      if (this->ui.replaceEdit->text().isEmpty()) {
         emit this->requestFindNext(this->ui.findEdit->text());
         return;
      }
      emit this->requestReplaceNext(this->ui.findEdit->text(), this->ui.replaceEdit->text());
   });
   QObject::connect(this->ui.buttonClose, &QPushButton::clicked, this, [this]() {
      emit this->requestClose();
   });
   QObject::connect(this->ui.buttonToggleReplace, &QPushButton::toggled, this, [this](bool checked) {
      this->ui.replaceRow->setVisible(checked);
      this->clearStatus();
      this->adjustSize();
   });

   auto* escape_shortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
   QObject::connect(escape_shortcut, &QShortcut::activated, this, [this]() {
      emit this->requestClose();
   });
}
void ScriptEditorFindReplaceBar::showFind(const QString& initial_find) {
   this->ui.buttonToggleReplace->setChecked(false);
   if (!initial_find.isEmpty())
      this->ui.findEdit->setText(initial_find);
   this->clearStatus();
   this->show();
   this->raise();
   this->focusFindField();
   this->adjustSize();
}
void ScriptEditorFindReplaceBar::showReplace(const QString& initial_find, const QString& initial_replace) {
   this->ui.buttonToggleReplace->setChecked(true);
   if (!initial_find.isEmpty())
      this->ui.findEdit->setText(initial_find);
   this->ui.replaceEdit->setText(initial_replace);
   this->clearStatus();
   this->show();
   this->raise();
   this->focusFindField();
   this->adjustSize();
}
QString ScriptEditorFindReplaceBar::findText() const {
   return this->ui.findEdit->text();
}
QString ScriptEditorFindReplaceBar::replaceText() const {
   return this->ui.replaceEdit->text();
}
void ScriptEditorFindReplaceBar::focusFindField() {
   this->ui.findEdit->setFocus();
   this->ui.findEdit->selectAll();
}
void ScriptEditorFindReplaceBar::setMatchCount(int count) {
   if (count < 0) {
      this->ui.matchCount->setText(QStringLiteral("--"));
      this->ui.matchCount->setStyleSheet(QStringLiteral(
         "QLabel {"
         "  color: #e0e6ef;"
         "  background-color: #2b3240;"
         "  border: 1px solid #5c6980;"
         "  border-radius: 6px;"
         "  padding: 1px 6px;"
         "  font-weight: 600;"
         "}"
      ));
      return;
   }
   if (count == 0) {
      this->ui.matchCount->setText(tr("0 found"));
      this->ui.matchCount->setStyleSheet(QStringLiteral(
         "QLabel {"
         "  color: #ffd9d7;"
         "  background-color: #5b2a2a;"
         "  border: 1px solid #d88787;"
         "  border-radius: 6px;"
         "  padding: 1px 6px;"
         "  font-weight: 700;"
         "}"
      ));
      return;
   }
   this->ui.matchCount->setText(tr("%1 found").arg(count));
   this->ui.matchCount->setStyleSheet(QStringLiteral(
      "QLabel {"
      "  color: #e6f7ff;"
      "  background-color: #1f4b61;"
      "  border: 1px solid #7bc8ef;"
      "  border-radius: 6px;"
      "  padding: 1px 6px;"
      "  font-weight: 600;"
      "}"
   ));
}
void ScriptEditorFindReplaceBar::setStatus(const QString& text, bool error) {
   if (text.isEmpty()) {
      this->clearStatus();
      return;
   }
   this->ui.statusLabel->setText(text);
   this->ui.statusLabel->setVisible(true);
   if (error) {
      this->ui.statusLabel->setStyleSheet(QStringLiteral(
         "QLabel {"
         "  color: #ffd9d7;"
         "  background-color: #5b2a2a;"
         "  border: 1px solid #d88787;"
         "  border-radius: 5px;"
         "  padding: 1px 6px;"
         "}"
      ));
      return;
   }
   this->ui.statusLabel->setStyleSheet(QStringLiteral(
      "QLabel {"
      "  color: #e6f7ff;"
      "  background-color: #1f4b61;"
      "  border: 1px solid #7bc8ef;"
      "  border-radius: 5px;"
      "  padding: 1px 6px;"
      "}"
   ));
}
void ScriptEditorFindReplaceBar::clearStatus() {
   this->ui.statusLabel->setText(QStringLiteral(" "));
   this->ui.statusLabel->setVisible(true);
   this->ui.statusLabel->setStyleSheet(QStringLiteral(
      "QLabel {"
      "  color: transparent;"
      "  background-color: transparent;"
      "  border: 1px solid transparent;"
      "  border-radius: 5px;"
      "  padding: 1px 6px;"
      "}"
   ));
}
