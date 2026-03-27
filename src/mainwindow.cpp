#include "mainwindow.h"
#include "utils.h"
#include <QtUiTools>
#include <QMessageBox>
#include <QPlainTextEdit>


MainWindow::MainWindow()
{
    QFile file("assets/gui_main.ui");
    file.open(QIODevice::ReadOnly);
    QUiLoader loader;
    m_mainwindow = loader.load(&file, this);
    file.close();

    m_mainwindow->setWindowIcon(QIcon("assets/icons/app.png"));
    m_mainwindow->installEventFilter(this);
    m_mainwindow->setWindowFilePath(QString::fromStdString(m_currentPrjName));

    m_menu_close = m_mainwindow->findChild<QAction*>("actionClose");
    m_menu_openProj = m_mainwindow->findChild<QAction*>("actionOpen_Project");
    m_menu_saveAsProj = m_mainwindow->findChild<QAction*>("actionSaveAs_Project");
    m_menu_saveProj = m_mainwindow->findChild<QAction*>("actionSave_Project");
    m_menu_autoExpand = m_mainwindow->findChild<QAction*>("actionAuto_Expand");
    m_menu_clearResults = m_mainwindow->findChild<QAction*>("actionClear_Results");
    m_menu_exportStart = m_mainwindow->findChild<QAction*>("actionExport_Start");
    m_menu_exportFinish = m_mainwindow->findChild<QAction*>("actionExport_Finish");
    m_menu_generateReport = m_mainwindow->findChild<QAction*>("actionGenerate_Report");
    m_menu_about = m_mainwindow->findChild<QAction*>("actionAbout");

    m_tw_testcase = m_mainwindow->findChild<QTreeWidget*>("tw_testcase");
    m_le_tree_uid = m_mainwindow->findChild<QLineEdit*>("le_tree_uid");
    m_cb_result = m_mainwindow->findChild<QComboBox*>("cb_result");
    m_ckb_tree_as_child = m_mainwindow->findChild<QCheckBox*>("ckb_tree_as_child");
    m_btn_tree_confirm = m_mainwindow->findChild<QPushButton*>("btn_tree_confirm");
    m_cb_tree_method = m_mainwindow->findChild<QComboBox*>("cb_tree_method");
    m_cb_tree_method->addItems({"Add Testcase", "Add Subfolder", "Follow", "Delete"});

    m_lb_uid = m_mainwindow->findChild<QLabel*>("lb_uid");
    m_le_name = m_mainwindow->findChild<QLineEdit*>("le_name");
    m_le_curr_proj = m_mainwindow->findChild<QLineEdit*>("le_curr_proj");
    m_te_comment = m_mainwindow->findChild<QTextEdit*>("te_comment");
    m_cb_testtype = m_mainwindow->findChild<QComboBox*>("cb_testtype");
    m_tw_testarea = m_mainwindow->findChild<QTableWidget*>("tw_testarea");
    m_btn_up = m_mainwindow->findChild<QPushButton*>("btn_up");
    m_btn_down = m_mainwindow->findChild<QPushButton*>("btn_down");
    m_btn_plus = m_mainwindow->findChild<QPushButton*>("btn_plus");
    m_btn_minus = m_mainwindow->findChild<QPushButton*>("btn_minus");
    m_btn_up = m_mainwindow->findChild<QPushButton*>("btn_up");
    m_btn_down = m_mainwindow->findChild<QPushButton*>("btn_down");
    m_btn_savecase = m_mainwindow->findChild<QPushButton*>("btn_savecase");

    m_sc_space = new QShortcut(QKeySequence(Qt::Key_Space), m_tw_testcase);
    m_sc_save_case_prj = new QShortcut(QKeySequence(tr("Ctrl+S")), m_tw_testarea);
    m_sc_open_prj = new QShortcut(QKeySequence(tr("Ctrl+O")), m_tw_testarea);

    connect(m_sc_space, SIGNAL(activated()), this, SLOT(displayTestCaseSafe()));
    connect(m_sc_save_case_prj, SIGNAL(activated()), this, SLOT(saveProject()));
    connect(m_sc_open_prj, SIGNAL(activated()), this, SLOT(openProject()));

    connect(m_tw_testcase, SIGNAL(clicked(QModelIndex)), this, SLOT(displayTestCase()));
    connect(m_menu_close, SIGNAL(triggered()), this, SLOT(closeApp()));
    connect(m_menu_openProj, SIGNAL(triggered()), this, SLOT(openProject()));
    connect(m_menu_saveProj, SIGNAL(triggered()), this, SLOT(saveProject()));
    connect(m_menu_saveAsProj, SIGNAL(triggered()), this, SLOT(saveAsProject()));
    connect(m_menu_clearResults, SIGNAL(triggered()), this, SLOT(clearResults()));
    connect(m_menu_autoExpand, SIGNAL(triggered()), this, SLOT(expandTestTree()));
    connect(m_menu_about, SIGNAL(triggered()), this, SLOT(showAbout()));
    connect(m_menu_exportStart, SIGNAL(triggered()), this, SLOT(startExportTP()));
    connect(m_menu_exportFinish, SIGNAL(triggered()), this, SLOT(finishExportTP()));
    connect(m_menu_generateReport, SIGNAL(triggered()), this, SLOT(printReport()));
    connect(m_btn_tree_confirm, SIGNAL(clicked()), this, SLOT(execTreeConfirm()));

    connect(m_cb_tree_method, SIGNAL(currentIndexChanged(int)), this, SLOT(updateConfirmGUI()));

    connect(m_btn_plus, SIGNAL(clicked()), this, SLOT(addTestCaseStep()));
    connect(m_btn_minus, SIGNAL(clicked()), this, SLOT(deleteTestCaseStep()));
    connect(m_btn_up, SIGNAL(clicked()), this, SLOT(moveTestStepUp()));
    connect(m_btn_down, SIGNAL(clicked()), this, SLOT(moveTestStepDown()));

    connect(m_btn_savecase, SIGNAL(clicked()), this, SLOT(saveProject()));
    connect(m_cb_result, SIGNAL(currentIndexChanged(int)), this, SLOT(updateResultGUI()));

    clearTestarea();
    initProject();

    m_mainwindow->show();
    showTestCaseTree(m_proj->data());


}

MainWindow::~MainWindow()
{

}


void MainWindow::startExportTP()
{
    b_exportStart = true;
    if(m_proj){
        showTestCaseTree(m_proj->data());
    }
}

void MainWindow::finishExportTP()
{
    if (!b_exportStart || !m_proj) return;

    Project *m_export_proj = new Project;
    *m_export_proj = *m_proj;
    int sz_tests = m_export_proj->data()->node_data.size();

    auto items = m_tw_testcase->findItems("", Qt::MatchContains | Qt::MatchRecursive);

    std::vector<std::string> uncheckedItems;
    for (auto &it : items)
    {
        if (!it->checkState(0)) {
            uncheckedItems.push_back(it->text(0).toStdString());
        }
    }

    for (auto &it : uncheckedItems)
    {
        int uid_unchecked = QString::fromStdString(it).split(":").at(0).toInt();
        m_export_proj->deleteNode(uid_unchecked);
    }

    if (uncheckedItems.size() == sz_tests)
    {
        delete m_export_proj;
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export Test Plan as"), QDir::homePath(), tr("Project File (*.qlpro)"));

    if (fileName.toStdString().empty()){
        return;
    }
    if(m_export_proj){
        m_export_proj->saveProject(getFileWithExtention(fileName.toStdString(), "qlpro").c_str());
    }
    delete m_export_proj;

    b_exportStart = false;
    showTestCaseTree(m_proj->data());
}



bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (object == this->m_mainwindow && event->type() == QEvent::Close)
    {
        qApp->quit();
    }
    return false;
}


void MainWindow::execTreeConfirm()
{
    std::string cf_option = m_cb_tree_method->currentText().toStdString();
    bool b_asChild = m_ckb_tree_as_child->isChecked();
    if (cf_option == "Follow" && m_tw_testcase->model()->rowCount())
    {
        if (!m_tw_testcase->selectionModel()->selectedIndexes().count()){
            return;
        }
        int uid_selected = m_tw_testcase->currentIndex().data().toString().split(":").at(0).toInt();
        int uid_target = m_le_tree_uid->text().toInt();
        if(!m_proj->followNode(uid_selected, uid_target, b_asChild)){
            QMessageBox::information(m_mainwindow, tr("Info"), tr("Follow operation failed: Nodes shall not follow each other "
                                                                  "within the same branch."));
            return;
        }
    }
    if (cf_option == "Add Subfolder" || cf_option == "Add Testcase")
    {
        NodeType nodetype = (cf_option == "Add Testcase") ? NodeType::eNodeTestCase : NodeType::eNodeSubFolder;
        int uid_target = m_le_tree_uid->text().toInt();
        if(!m_proj->addNode(uid_target, nodetype, b_asChild)){
            QMessageBox::information(m_mainwindow, tr("Info"), tr("Failed to add a new Node."));
            return;
        }
    }
    if (cf_option == "Delete")
    {
        if (!m_tw_testcase->selectionModel()->selectedIndexes().count()){
            return;
        }
        int uid_selected = m_tw_testcase->currentIndex().data().toString().split(":").at(0).toInt();
        if (!isNodeDeletable(uid_selected)){
            QMessageBox::information(m_mainwindow, tr("Info"), tr("Deletion failed: there must be at least one node remaining."));
            return;
        }
        QString msg = "Do you really want to delete [>> UID-" + QString::number(uid_selected) + " <<] including its subcases?";
        QMessageBox::StandardButton qReplay = QMessageBox::question(m_mainwindow, "Warning", msg);
        if (qReplay != QMessageBox::Yes){
            return;
        }
        if(!m_proj->deleteNode(uid_selected)){
            QMessageBox::information(m_mainwindow, tr("Info"), tr("Deletion failed!"));
            return;
        }
    }
    showTestCaseTree(m_proj->data());
}

void MainWindow::openProject()
{
    if (!m_currentPrjName.empty()) {
        QMessageBox::StandardButton qReply = QMessageBox::question(m_mainwindow, "Warning",
                                                                   "[!] Current project will be discarded, do you want to continue?");
        if (qReply != QMessageBox::Yes){
            return;
        }
    }

    QString fileName = QFileDialog::getOpenFileName(this,
                                            tr("Open Project"), QDir::homePath(), tr("Project File (*.qlpro)"));
    if (fileName.isEmpty() || fileName.isNull()){
        return;
    }
    delete m_proj;
    m_proj = new Project;
    if (!m_proj->openProject(fileName.toStdString().c_str())){
        return;
    }
    clearTestarea();
    showTestCaseTree(m_proj->data());

    m_currentPrjName = getFileWithExtention(fileName.toStdString(), "qlpro");
    m_le_curr_proj->setReadOnly(true);
    m_le_curr_proj->setText(QString::fromStdString(m_currentPrjName));
}

void MainWindow::saveAsProject()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Project as"), QDir::homePath(), tr("Project File (*.qlpro)"));

    if (fileName.toStdString().empty())
    {
        return;
    }

    updateTestCase();
    if(m_proj){
        m_proj->saveProject(getFileWithExtention(fileName.toStdString(), "qlpro").c_str());
    }
    m_currentPrjName = getFileWithExtention(fileName.toStdString(), "qlpro");
    m_le_curr_proj->setReadOnly(true);
    m_le_curr_proj->setText(QString::fromStdString(m_currentPrjName));
}

void MainWindow::saveProject()
{
    if(m_currentPrjName.empty())
    {
        saveAsProject();
    }    
    else if(m_proj){
        updateTestCase();
        m_proj->saveProject(getFileWithExtention(m_currentPrjName, "qlpro").c_str());
    }
    m_le_curr_proj->setText(QString::fromStdString(m_currentPrjName));
}


void MainWindow::clearResults()
{
    QMessageBox::StandardButton qReply = QMessageBox::question(m_mainwindow, "Warning",
                                                                "[!] All test results are going to be cleaned up, do you want to continue?");
    if (qReply != QMessageBox::Yes){
        return;
    }

    for (auto & data : m_proj->data()->node_data)
    {
        data.testResult = "Neutral";
    }
    clearTestarea();
    showTestCaseTree(m_proj->data());
}

void MainWindow::closeApp()
{
    m_mainwindow->close();
    qApp->quit();
}

void MainWindow::showAbout()
{
    QDialog * aboutDialog = new QDialog();
    aboutDialog->setFixedSize(QSize(250,250));
    QPushButton * ok_btn = new QPushButton("OK");
    connect(ok_btn, SIGNAL(clicked(bool)), aboutDialog, SLOT(close()));

    QTextEdit * txt = new QTextEdit();
    txt->setReadOnly(true);
    txt->setAlignment(Qt::AlignCenter);
    txt->append("V2.7.3\n");
    txt->append("A lightweight management tool for manual testing.\n");
    txt->append("Copyright 2025 - 2026 S. Wang\n");

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(txt);
    layout->addWidget(ok_btn);

    aboutDialog->setAttribute(Qt::WA_DeleteOnClose);
    aboutDialog->setLayout(layout);
    aboutDialog->setWindowIcon(QIcon("assets/icons/app.png"));
    aboutDialog->setWindowTitle("About QoolTester");
    aboutDialog->show();
}

void MainWindow::displayTestCaseSafe()
{
    if(!m_tw_testcase->selectionModel()->selectedIndexes().count())
    {
        return;
    } else {
        displayTestCase();
    }
}

void MainWindow::displayTestCase()
{
    if (m_tw_testcase->currentIndex().data().isNull()) return;

    QString uid = m_tw_testcase->currentIndex().data().toString().split(':').at(0);
    m_lb_uid->setText(uid);
    QString name = m_tw_testcase->currentIndex().data().toString().section(':',1).trimmed();
    m_le_name->setText(name);

    Node_data_t data_selected;
    for (auto & data : m_proj->data()->node_data)
    {
        if (data.uid == std::stoi(uid.toStdString().c_str())){
            data_selected = data;
        }
    }
    if (!data_selected.testType.empty() &&
        m_cb_testtype->findText(QString::fromStdString(data_selected.testType))<0)
    {
        m_cb_testtype->addItem(QString::fromStdString(data_selected.testType));
    }
    m_cb_testtype->setCurrentText(QString::fromStdString(data_selected.testType));
    m_cb_result->setCurrentText(QString::fromStdString(data_selected.testResult));
    m_te_comment->setText(QString::fromStdString(data_selected.comment));

    m_tw_testarea->setItemDelegateForColumn(0, new MultilineDelegate(this));
    m_tw_testarea->setItemDelegateForColumn(1, new MultilineDelegate(this));
    m_tw_testarea->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    m_tw_testarea->setRowCount(0);
    for (int i_row=0; i_row<data_selected.testdata.size(); i_row++){
        m_tw_testarea->insertRow(i_row);

        QTableWidgetItem* m_stepIn = new QTableWidgetItem;
        QTableWidgetItem* m_stepOut = new QTableWidgetItem;
        m_stepIn->setText(QString::fromStdString(std::get<0>(data_selected.testdata[i_row])));
        m_stepOut->setText(QString::fromStdString(std::get<1>(data_selected.testdata[i_row])));

        m_tw_testarea->setItem(i_row, 0, m_stepIn);
        m_tw_testarea->setItem(i_row, 1, m_stepOut);
    }
}

void MainWindow::updateTestCaseTree()
{
    std::string testRs = m_cb_result->currentText().toStdString();
    if (testRs== "Neutral") {
        m_tw_testcase->selectionModel()->model()->setData(m_tw_testcase->selectionModel()->currentIndex(), QBrush(Qt::NoBrush), Qt::BackgroundRole);
    }
    else if (testRs == "Retest") {
        m_tw_testcase->selectionModel()->model()->setData(m_tw_testcase->selectionModel()->currentIndex(), QBrush(QColor(196, 183, 63)), Qt::BackgroundRole);
    }
    else if (testRs == "Pass") {
        m_tw_testcase->selectionModel()->model()->setData(m_tw_testcase->selectionModel()->currentIndex(), QBrush(QColor(122, 201, 141)), Qt::BackgroundRole);
    }
    else if (testRs == "Fail") {
        m_tw_testcase->selectionModel()->model()->setData(m_tw_testcase->selectionModel()->currentIndex(), QBrush(QColor(222, 144, 151)), Qt::BackgroundRole);
    }

    // update the test name as well
    std::string full_name = m_lb_uid->text().toStdString()+": "+m_le_name->text().toStdString();
    m_tw_testcase->selectionModel()->model()->setData(m_tw_testcase->selectionModel()->currentIndex(), QString::fromStdString(full_name));
}

bool MainWindow::showTestCaseTree(Project_data_t * proj_data)
{
    m_tw_testcase->clear();

    std::vector<QTreeWidgetItem*> topLevelItems;
    QTreeWidgetItem * lastTopLevelItem;
    std::vector<QTreeWidgetItem*> lastItemPerLevel(proj_data->max_level+1);

    for(auto & elm : proj_data->node_data)
    {
        QTreeWidgetItem * testItem = new QTreeWidgetItem();
        testItem->setText(0, QString::fromStdString(elm.full_name));
        lastItemPerLevel[elm.level] = testItem;

        if (elm.level == 1) {
            topLevelItems.push_back(testItem);
            lastTopLevelItem = testItem;
        }
        if (elm.level == 2) {
            lastTopLevelItem->addChild(testItem);
        }
        if (elm.level > 2) {
            lastItemPerLevel[elm.level-1]->addChild(testItem);
        }
        if(elm.isTestCase){
            testItem->setIcon(0, QIcon("assets/icons/testcase.png"));
        } else {
            testItem->setIcon(0, QIcon("assets/icons/folder.png"));
        }
        if (elm.testResult == "Neutral") {
            QBrush tv_brush;
            tv_brush.setStyle(Qt::NoBrush);
            testItem->setBackground(0, tv_brush);
        }
        else if (elm.testResult == "Retest") {
            QBrush tv_brush(QColor(196, 183, 63), Qt::SolidPattern);
            testItem->setBackground(0, tv_brush);
        }
        else if (elm.testResult == "Pass") {
            QBrush tv_brush(QColor(122,201,141), Qt::SolidPattern);
            testItem->setBackground(0, tv_brush);
        }
        else if (elm.testResult == "Fail") {
            QBrush tv_brush(QColor(222,144,151), Qt::SolidPattern);
            testItem->setBackground(0, tv_brush);
        }
        if (b_exportStart)
        {
            testItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
            testItem->setCheckState(0, Qt::Unchecked);
        }
    }

    for (auto *item: topLevelItems){
        m_tw_testcase->addTopLevelItem(item);
    }
    m_tw_testcase->model()->setHeaderData(0, Qt::Horizontal, "TreeView");

    expandTestTree();

    return true;
}

void MainWindow::expandTestTree()
{
    if (m_menu_autoExpand->isChecked())
    {
        m_tw_testcase->expandAll();
    } else {
        m_tw_testcase->collapseAll();
    }
}

void MainWindow::addTestCaseStep()
{
    if (!m_tw_testcase->selectionModel()->selectedIndexes().count()){
        return;
    }
    QString uid = m_tw_testcase->currentIndex().data().toString().split(':').at(0);
    if (!m_proj->addCaseStep(uid.toInt())){
        return;
    }
    m_tw_testarea->insertRow(m_tw_testarea->rowCount());
}

void MainWindow::deleteTestCaseStep()
{
    if (!m_tw_testcase->selectionModel()->selectedIndexes().count()){
        return;
    }
    if (m_tw_testarea->selectionModel()->selectedIndexes().count() != 1){
        return;
    }
    QString uid = m_tw_testcase->currentIndex().data().toString().split(':').at(0);
    if (!m_proj->deleteCaseStep(uid.toInt(), m_tw_testarea->currentRow()))
    {
        return;
    }
    m_tw_testarea->removeRow(m_tw_testarea->currentRow());
}

void MainWindow::moveTestStepUp()
{
    if (!m_tw_testcase->selectionModel()->selectedIndexes().count()){
        return;
    }
    if (m_tw_testarea->selectionModel()->selectedIndexes().count() != 1){
        return;
    }
    QString uid = m_tw_testcase->currentIndex().data().toString().split(':').at(0);
    if (!m_proj->moveUpCaseStep(uid.toInt(), m_tw_testarea->currentRow())){
        return;
    }
    int current_row = m_tw_testarea->currentRow();
    for (int i_col=0; i_col<2; i_col++){
        QTableWidgetItem *item = m_tw_testarea->takeItem(current_row,i_col);
        QTableWidgetItem *itemAbove = m_tw_testarea->takeItem(current_row-1,i_col);
        m_tw_testarea->setItem(current_row,i_col,itemAbove);
        m_tw_testarea->setItem(current_row-1,i_col,item);
    }
}

void MainWindow::moveTestStepDown()
{
    if (!m_tw_testcase->selectionModel()->selectedIndexes().count()){
        return;
    }
    if (m_tw_testarea->selectionModel()->selectedIndexes().count() != 1){
        return;
    }
    QString uid = m_tw_testcase->currentIndex().data().toString().split(':').at(0);
    if(!m_proj->moveDownCaseStep(uid.toInt(), m_tw_testarea->rowCount(), m_tw_testarea->currentRow())){
        return;
    }
    int current_row = m_tw_testarea->currentRow();
    for (int i_col=0; i_col<2; i_col++){
        QTableWidgetItem *item = m_tw_testarea->takeItem(current_row,i_col);
        QTableWidgetItem *itemAbove = m_tw_testarea->takeItem(current_row+1,i_col);
        m_tw_testarea->setItem(current_row,i_col,itemAbove);
        m_tw_testarea->setItem(current_row+1,i_col,item);
    }
}

void MainWindow::updateTestCase()
{
    if (!m_tw_testcase->selectionModel()->selectedIndexes().count()){
        return;
    }
    int uid_selected = m_tw_testcase->currentIndex().data().toString().split(":").at(0).toInt();
    int idx_got;
    m_proj->getVecIndex(uid_selected, idx_got);

    Node_data_t mod_data;
    mod_data.uid = m_lb_uid->text().toInt();
    mod_data.name = m_le_name->text().toStdString();
    mod_data.full_name = m_lb_uid->text().toStdString()+": "+mod_data.name;
    mod_data.testType = m_cb_testtype->currentText().toStdString();
    mod_data.testResult = m_cb_result->currentText().toStdString();
    mod_data.isTestCase = m_proj->data()->node_data[idx_got].isTestCase;
    mod_data.comment = m_te_comment->toPlainText().toStdString();
    mod_data.level = m_proj->data()->node_data[idx_got].level;

    for (int i_step=0; i_step < m_tw_testarea->rowCount(); i_step++){
        // Qt6 has Segfault-Bug, if some cells remain empty aka. tablewidget.item.text() causes crash.
        std::string stepIn=(m_tw_testarea->item(i_step,0)==nullptr)?"":m_tw_testarea->item(i_step,0)->text().toStdString();
        std::string stepOut=(m_tw_testarea->item(i_step,1)==nullptr)?"":m_tw_testarea->item(i_step,1)->text().toStdString();
        mod_data.testdata.push_back(std::make_tuple(stepIn, stepOut));
    }
    m_proj->data()->node_data[idx_got] = mod_data;

    updateTestCaseTree();
}


void MainWindow::initProject()
{
    b_exportStart = false;

    m_proj = new Project;
    Node_data_t node_add {99, "Default", "99: Default", 1, true, "Acceptance Test", "Neutral", "", {{"", ""}}};
    m_proj->data()->node_data.push_back(node_add);
    m_proj->data()->max_level = 1;
}

bool MainWindow::isNodeDeletable(int uid_select)
{
    bool b_match = true;
    if (!m_proj->compareSubMainEqual(uid_select, b_match)){
        return false;   // compare operation has failed
    }
    if (!b_match){
        return true;
    }
    return false;
}

void MainWindow::clearTestarea()
{
    m_cb_testtype->clear();
    m_cb_testtype->addItems({"Acceptance Test", "Ad hoc Test", "Compatibility Test", "Destructive Test", "Exploratory Test", "Functional Test", "Performance Test",
                            "Regression Test", "Smoke Test", "Unit Test", "Usability Test"});
    m_cb_result->clear();
    m_cb_result->addItems({"Neutral", "Pass", "Fail", "Retest"});
    updateResultGUI();

    m_lb_uid->clear();
    m_le_name->clear();
    m_te_comment->clear();
    m_tw_testarea->clear();
    m_tw_testarea->setRowCount(0);

    m_tw_testarea->setColumnCount(2);
    QTableWidgetItem *headerIn = new QTableWidgetItem();
    headerIn->setText("Test Steps:");
    QTableWidgetItem *headerOut = new QTableWidgetItem();
    headerOut->setText("Expected Result:");
    m_tw_testarea->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tw_testarea->setHorizontalHeaderItem(0, headerIn);
    m_tw_testarea->setHorizontalHeaderItem(1, headerOut);
    m_btn_up->setIcon(QIcon("assets/icons/up.png"));
    m_btn_down->setIcon(QIcon("assets/icons/down.png"));
    m_btn_plus->setIcon(QIcon("assets/icons/plus.png"));
    m_btn_minus->setIcon(QIcon("assets/icons/minus.png"));
}

void MainWindow::updateConfirmGUI()
{
    if (m_cb_tree_method->currentText() == "Delete"){
        m_le_tree_uid->setEnabled(false);
    } else {
        m_le_tree_uid->setEnabled(true);
    }
}


void MainWindow::updateResultGUI()
{
    if (m_cb_result->currentText() == "Neutral"){
        m_cb_result->setStyleSheet("");
    }
    else if (m_cb_result->currentText() == "Retest"){
        m_cb_result->setStyleSheet("QComboBox { background-color: rgb(196, 183, 63);}");
    }
    else if (m_cb_result->currentText() == "Pass"){
        m_cb_result->setStyleSheet("QComboBox { background-color: rgb(122,201,141);}");
    }
    else if (m_cb_result->currentText() == "Fail"){
        m_cb_result->setStyleSheet("QComboBox { background-color:  rgb(222,144,151);}");
    }
}

void MainWindow::printReport()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Report as"), QDir::homePath(), tr("Report File (*.pdf)"));
    if (fileName.toStdString().empty()){
        return;
    }
    m_rptr = new Reporter(m_proj->data(), getFileWithExtention(fileName.toStdString(), "pdf"));
}



QWidget *MultilineDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return new QPlainTextEdit(parent);
}

void MultilineDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (auto *textEdit = qobject_cast<QPlainTextEdit *>(editor)) {
        textEdit->setPlainText(index.data(Qt::EditRole).toString());
    } else {
        QItemDelegate::setEditorData(editor, index);
    }
}

void MultilineDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (auto *textEdit = qobject_cast<QPlainTextEdit *>(editor)) {
        model->setData(index, textEdit->toPlainText());
    } else {
        QItemDelegate::setModelData(editor, model, index);
    }
}

