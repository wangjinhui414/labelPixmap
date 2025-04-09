#ifndef SIMPLEINHERITANCE_H
#define SIMPLEINHERITANCE_H

#include <QObject>
#include <QLabel>

class ImageLabel : public QLabel {
    Q_OBJECT

public:
    explicit ImageLabel(QWidget *parent = nullptr) : QLabel(parent) {
        this->setAlignment(Qt::AlignCenter); // 图片居中显示
    }

    void setPixmap(const QPixmap &pixmap) {
        originalPixmap = pixmap; // 保存原始图片
        QLabel::setPixmap(scalePixmap()); // 设置缩放后的图片
    }

protected:
    void resizeEvent(QResizeEvent *event) override {
        QLabel::resizeEvent(event);
        if (!originalPixmap.isNull()) {
            // 窗口大小变化时，重新缩放图片
            QLabel::setPixmap(scalePixmap());
        }
    }

private:
    QPixmap originalPixmap; // 原始图片

    // 按 QLabel 的实际尺寸缩放图片
    QPixmap scalePixmap() const {
        return originalPixmap.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
};



#endif // SIMPLEINHERITANCE_H
