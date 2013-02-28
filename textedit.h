#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QPlainTextEdit>


class TextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit TextEdit(QWidget *parent = 0);
    
    void setIndentSize(int indentSize) {this->_indentSize = indentSize;}
    int  indentSize() const {return this->_indentSize;}

    void setIndentWithSpace(bool indentWithSpace)  {this->_indentWithSpace = indentWithSpace;}
    bool indentWithSpace() const { return _indentWithSpace; }

    void setAutoIndent(bool autoIndent) {this->_autoIndent = autoIndent;}
    bool autoIndent() const {return this->_autoIndent;}

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    void keyPressEvent(QKeyEvent *e);
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    int  _indentSize;
    bool _indentWithSpace;
    bool _autoIndent;

    QWidget *lineNumberArea;
};

#endif // TEXTEDIT_H
