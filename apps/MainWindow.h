#pragma once

#include <QMainWindow>
#include <QTranslator>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

namespace blokusUi
{
    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        MainWindow(QWidget* _parent = nullptr);
        ~MainWindow();

    protected:
        virtual void changeEvent(QEvent* _event) override;

    private:
        void setupActions();
        void setupConnections();
        void setupTranslator(const QLocale& _locale);

    private slots:
        void updateLanguage();
        void about();

    private:
        Ui::MainWindow* m_ui;
        QTranslator m_translator;
    };
}
