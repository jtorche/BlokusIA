#pragma once

#include <QMainWindow>

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
        void setupConnections() const;

    private slots:
        void updateLanguage() const;
        void updateTheme() const;
        void about();

    private:
        Ui::MainWindow* m_ui;
    };
}
