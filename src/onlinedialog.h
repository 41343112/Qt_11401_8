#ifndef ONLINEDIALOG_H
#define ONLINEDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QCheckBox>
#include <QMap>

class OnlineDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OnlineDialog(QWidget *parent = nullptr);
    ~OnlineDialog();

    QMap<QString, bool> getGameModes() const;

private:
    void setupUI();

    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    
    // Game mode checkboxes
    QCheckBox* m_standardModeCheckbox;
    QCheckBox* m_rapidModeCheckbox;
    QCheckBox* m_blitzModeCheckbox;
    QCheckBox* m_handicapModeCheckbox;
    QCheckBox* m_customRulesCheckbox;
};

#endif // ONLINEDIALOG_H
