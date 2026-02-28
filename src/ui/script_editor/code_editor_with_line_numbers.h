#pragma once
#include <QPlainTextEdit>
#include <QColor>

class QPaintEvent;
class QResizeEvent;

class CodeEditorWithLineNumbers : public QPlainTextEdit {
   Q_OBJECT

   public:
      explicit CodeEditorWithLineNumbers(QWidget* parent = nullptr);

      int lineNumberAreaWidth() const;
      void lineNumberAreaPaintEvent(QPaintEvent* event);
      void setLineNumberAreaColors(const QColor& background, const QColor& normal, const QColor& current);

   protected:
      bool event(QEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;

   private:
      void updateLineNumberAreaWidth(int);
      void updateLineNumberArea(const QRect& rect, int dy);
      QColor effectiveLineNumberAreaBackground() const;
      QColor effectiveLineNumberColor() const;
      QColor effectiveCurrentLineNumberColor() const;

      QWidget* _lineNumberArea = nullptr;
      QColor _lineNumberAreaBackground;
      QColor _lineNumberColor;
      QColor _currentLineNumberColor;
};
