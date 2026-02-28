#include "code_editor_with_line_numbers.h"
#include <QEvent>
#include <QPainter>
#include <QTextBlock>

namespace {
   class CodeEditorLineNumberArea : public QWidget {
      public:
         explicit CodeEditorLineNumberArea(CodeEditorWithLineNumbers* editor) : QWidget(editor), _editor(editor) {}

         QSize sizeHint() const override {
            return QSize(this->_editor->lineNumberAreaWidth(), 0);
         }

      protected:
         void paintEvent(QPaintEvent* event) override {
            this->_editor->lineNumberAreaPaintEvent(event);
         }

      private:
         CodeEditorWithLineNumbers* _editor = nullptr;
   };
}

CodeEditorWithLineNumbers::CodeEditorWithLineNumbers(QWidget* parent) : QPlainTextEdit(parent) {
   this->_lineNumberArea = new CodeEditorLineNumberArea(this);

   QObject::connect(this, &QPlainTextEdit::blockCountChanged, this, &CodeEditorWithLineNumbers::updateLineNumberAreaWidth);
   QObject::connect(this, &QPlainTextEdit::updateRequest, this, &CodeEditorWithLineNumbers::updateLineNumberArea);
   QObject::connect(this, &QPlainTextEdit::cursorPositionChanged, this, [this]() {
      if (this->_lineNumberArea)
         this->_lineNumberArea->update();
   });

   this->updateLineNumberAreaWidth(0);
}

int CodeEditorWithLineNumbers::lineNumberAreaWidth() const {
   int digits = 1;
   int max    = qMax(1, this->blockCount());
   while (max >= 10) {
      max /= 10;
      ++digits;
   }
   return 8 + this->fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void CodeEditorWithLineNumbers::setLineNumberAreaColors(const QColor& background, const QColor& normal, const QColor& current) {
   this->_lineNumberAreaBackground = background;
   this->_lineNumberColor = normal;
   this->_currentLineNumberColor = current;
   if (this->_lineNumberArea)
      this->_lineNumberArea->update();
}

QColor CodeEditorWithLineNumbers::effectiveLineNumberAreaBackground() const {
   if (this->_lineNumberAreaBackground.isValid())
      return this->_lineNumberAreaBackground;
   QColor background = this->palette().alternateBase().color();
   if (!background.isValid())
      background = this->palette().base().color().darker(105);
   return background;
}

QColor CodeEditorWithLineNumbers::effectiveLineNumberColor() const {
   if (this->_lineNumberColor.isValid())
      return this->_lineNumberColor;
   QColor color = this->palette().text().color();
   if (!color.isValid())
      color = QColor(Qt::white);
   color.setAlpha(170);
   if (color.alpha() < 120)
      color.setAlpha(170);
   return color;
}

QColor CodeEditorWithLineNumbers::effectiveCurrentLineNumberColor() const {
   if (this->_currentLineNumberColor.isValid())
      return this->_currentLineNumberColor;
   QColor color = this->palette().text().color();
   if (!color.isValid())
      color = QColor(Qt::white);
   return color;
}

void CodeEditorWithLineNumbers::lineNumberAreaPaintEvent(QPaintEvent* event) {
   QPainter painter(this->_lineNumberArea);

   QColor background = this->effectiveLineNumberAreaBackground();
   painter.fillRect(event->rect(), background);

   QColor currentColor = this->effectiveCurrentLineNumberColor();
   QColor normalColor  = this->effectiveLineNumberColor();

   QTextBlock block = this->firstVisibleBlock();
   int blockNumber  = block.blockNumber();
   int top = qRound(this->blockBoundingGeometry(block).translated(this->contentOffset()).top());
   int bottom = top + qRound(this->blockBoundingRect(block).height());

   while (block.isValid() && top <= event->rect().bottom()) {
      if (block.isVisible() && bottom >= event->rect().top()) {
         const bool isCurrent = blockNumber == this->textCursor().blockNumber();
         const QString number = QString::number(blockNumber + 1);

         QFont lineFont = this->font();
         lineFont.setBold(isCurrent);
         painter.setFont(lineFont);
         painter.setPen(isCurrent ? currentColor : normalColor);
         painter.drawText(
            0,
            top,
            this->_lineNumberArea->width() - 4,
            this->fontMetrics().height(),
            Qt::AlignRight | Qt::AlignVCenter,
            number
         );
      }

      block = block.next();
      top = bottom;
      bottom = top + qRound(this->blockBoundingRect(block).height());
      ++blockNumber;
   }
}

bool CodeEditorWithLineNumbers::event(QEvent* event) {
   bool handled = QPlainTextEdit::event(event);
   if (event) {
      switch (event->type()) {
         case QEvent::FontChange:
         case QEvent::PaletteChange:
         case QEvent::StyleChange:
            this->updateLineNumberAreaWidth(0);
            if (this->_lineNumberArea)
               this->_lineNumberArea->update();
            break;
         default:
            break;
      }
   }
   return handled;
}

void CodeEditorWithLineNumbers::resizeEvent(QResizeEvent* event) {
   QPlainTextEdit::resizeEvent(event);

   if (!this->_lineNumberArea)
      return;

   QRect cr = this->contentsRect();
   this->_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), this->lineNumberAreaWidth(), cr.height()));
}

void CodeEditorWithLineNumbers::updateLineNumberAreaWidth(int) {
   this->setViewportMargins(this->lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditorWithLineNumbers::updateLineNumberArea(const QRect& rect, int dy) {
   if (!this->_lineNumberArea)
      return;

   if (dy)
      this->_lineNumberArea->scroll(0, dy);
   else
      this->_lineNumberArea->update(0, rect.y(), this->_lineNumberArea->width(), rect.height());

   if (rect.contains(this->viewport()->rect()))
      this->updateLineNumberAreaWidth(0);
}
