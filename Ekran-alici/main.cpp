#include <QApplication>
#include <QMainWindow>
#include <QScreen>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QMouseEvent>
#include <QPainter>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>

class EkranGoruntusuWidget : public QWidget {
    Q_OBJECT

public:
    EkranGoruntusuWidget() : secimAktif(false) {
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setCursor(Qt::CrossCursor);
    }

protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            baslangicNoktasi = event->pos();
            bitisNoktasi = baslangicNoktasi;
            secimAktif = true;
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if (secimAktif) {
            bitisNoktasi = event->pos();
            update();
        }
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton && secimAktif) {
            secimAktif = false;
            ekranGoruntusuAl();
            close();
        }
    }

    void paintEvent(QPaintEvent *) override {
        if (secimAktif) {
            QPainter painter(this);
            painter.setPen(QPen(Qt::blue, 2, Qt::DashLine));
            painter.setBrush(QBrush(QColor(0, 0, 0, 0)));
            painter.drawRect(QRect(baslangicNoktasi, bitisNoktasi));
        }
    }

private:
    QPoint baslangicNoktasi, bitisNoktasi;
    bool secimAktif;

    void ekranGoruntusuAl() {
        QRect rect(QPoint(qMin(baslangicNoktasi.x(), bitisNoktasi.x()), qMin(baslangicNoktasi.y(), bitisNoktasi.y())),
                   QPoint(qMax(baslangicNoktasi.x(), bitisNoktasi.x()), qMax(baslangicNoktasi.y(), bitisNoktasi.y())));
        QScreen *screen = QGuiApplication::primaryScreen();
        QPixmap ekranGoruntusu = screen->grabWindow(0, rect.x(), rect.y(), rect.width(), rect.height());

        emit ekranGoruntusuYakalanan(ekranGoruntusu);
    }

signals:
    void ekranGoruntusuYakalanan(const QPixmap &ekranGoruntusu);
};

class AnaPencere : public QMainWindow {
    Q_OBJECT

public:
    AnaPencere() {
        setWindowTitle("Ekran Görüntüsü Uygulaması");
        setMinimumSize(800, 600);

        QWidget *merkezWidget = new QWidget(this);
        setCentralWidget(merkezWidget);
        QVBoxLayout *duzen = new QVBoxLayout(merkezWidget);

        QHBoxLayout *butonDuzeni = new QHBoxLayout();
        QPushButton *btnKopyala = new QPushButton("Kopyala", this);
        QPushButton *btnKapat = new QPushButton("Kapat", this);
        QPushButton *btnYenidenAl = new QPushButton("Yeniden Al", this);
        QPushButton *btnKaydet = new QPushButton("Kaydet", this);

        butonDuzeni->addWidget(btnKopyala);
        butonDuzeni->addWidget(btnKapat);
        butonDuzeni->addWidget(btnYenidenAl);
        butonDuzeni->addWidget(btnKaydet);
        duzen->addLayout(butonDuzeni);

        ekranGoruntusuEtiketi = new QLabel("Ekran görüntüsü burada gösterilecek", this);
        duzen->addWidget(ekranGoruntusuEtiketi);

        connect(btnKapat, &QPushButton::clicked, this, &QMainWindow::close);
        connect(btnYenidenAl, &QPushButton::clicked, this, &AnaPencere::baslatEkranGoruntusu);
        connect(btnKaydet, &QPushButton::clicked, this, &AnaPencere::kaydetEkranGoruntusu);
        connect(btnKopyala, &QPushButton::clicked, this, &AnaPencere::kopyalaEkranGoruntusu);

        ekranGoruntusu = QPixmap();

        baslatEkranGoruntusu();
    }

private:
    QPixmap ekranGoruntusu;
    QLabel *ekranGoruntusuEtiketi;

    void baslatEkranGoruntusu() {
        EkranGoruntusuWidget *ekranGoruntusuWidget = new EkranGoruntusuWidget();
        connect(ekranGoruntusuWidget, &EkranGoruntusuWidget::ekranGoruntusuYakalanan, this, &AnaPencere::gosterEkranGoruntusu);
        ekranGoruntusuWidget->showFullScreen();
    }

    void gosterEkranGoruntusu(const QPixmap &ekranGoruntusu) {
        this->ekranGoruntusu = ekranGoruntusu;
        ekranGoruntusuEtiketi->setPixmap(ekranGoruntusu.scaled(ekranGoruntusuEtiketi->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    void kaydetEkranGoruntusu() {
        if (!ekranGoruntusu.isNull()) {
            QString dosyaAdi = QFileDialog::getSaveFileName(this, "Kaydet", "", "PNG Dosyası (*.png)");
            if (!dosyaAdi.isEmpty()) {
                if (!ekranGoruntusu.save(dosyaAdi, "PNG")) {
                    QMessageBox::warning(this, "Hata", "Ekran görüntüsü kaydedilemedi!");
                }
            }
        }
    }

    void kopyalaEkranGoruntusu() {
        if (!ekranGoruntusu.isNull()) {
            QClipboard *panoya = QApplication::clipboard();
            panoya->setPixmap(ekranGoruntusu);
        }
    }
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication uygulama(argc, argv);
    AnaPencere pencere;
    pencere.show();
    return uygulama.exec();
}
