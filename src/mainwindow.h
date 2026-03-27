#pragma once
#include <QtWidgets>
#include <QShortcut>
#include "project.h"
#include "reporter.h"

#include <QItemDelegate>


class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    bool showTestCaseTree(Project_data_t * proj_data);
    void updateTestCaseTree();
    void initProject();
    void clearTestarea();
    bool isNodeDeletable(int uid_select);


public slots:
    void expandTestTree();
    void displayTestCase();
    void displayTestCaseSafe();
    void closeApp();
    void openProject();
    void saveProject();
    void saveAsProject();
    void clearResults();
    void showAbout();
    void execTreeConfirm();
    void updateConfirmGUI();

    void addTestCaseStep();
    void deleteTestCaseStep();
    void moveTestStepUp();
    void moveTestStepDown();

    void updateTestCase();
    void updateResultGUI();

    void startExportTP();
    void finishExportTP();

    void printReport();


private:
    bool eventFilter(QObject *object, QEvent *event);

    QAction * m_menu_close;
    QAction * m_menu_openProj;
    QAction * m_menu_closeProj;
    QAction * m_menu_saveProj;
    QAction * m_menu_saveAsProj;
    QAction * m_menu_autoExpand;
    QAction * m_menu_clearResults;
    QAction * m_menu_exportStart;
    QAction * m_menu_exportFinish;
    QAction * m_menu_generateReport;
    QAction * m_menu_about;

    QWidget * m_mainwindow;
    QTreeWidget* m_tw_testcase;
    QLabel* m_lb_uid;
    QLineEdit* m_le_name;
    QLineEdit* m_le_curr_proj;
    QTextEdit* m_te_comment;
    QComboBox* m_cb_testtype;
    QTableWidget* m_tw_testarea;
    QPushButton* m_btn_up;
    QPushButton* m_btn_down;
    QPushButton* m_btn_plus;
    QPushButton* m_btn_minus;
    QPushButton* m_btn_savecase;

    QComboBox* m_cb_tree_method;
    QComboBox* m_cb_result;
    QLineEdit* m_le_tree_uid;
    QCheckBox* m_ckb_tree_as_child;
    QPushButton* m_btn_tree_confirm;

    QShortcut* m_sc_space;
    QShortcut* m_sc_open_prj;
    QShortcut* m_sc_save_case_prj;

    Project * m_proj;
    std::string m_currentPrjName;

    bool b_exportStart;

    Reporter * m_rptr;

};


class MultilineDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    using QItemDelegate::QItemDelegate;

public:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};
