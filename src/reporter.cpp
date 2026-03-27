#include "reporter.h"
#include <QString>
#include <QPrinter>
#include <chrono>
#include <ctime>


Reporter::Reporter(Project_data_t * proj, std::string reportPath) : m_proj(proj), m_reportPath(reportPath)
{
    m_commentReport = new QDialog();
    m_commentReport->setFixedSize(QSize(500,300));
    QPushButton * ok_btn = new QPushButton("OK");

    m_txt = new QPlainTextEdit();
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_txt);
    layout->addWidget(ok_btn);

    m_commentReport->setLayout(layout);
    m_commentReport->setWindowIcon(QIcon("assets/icons/app.png"));
    m_commentReport->setWindowTitle("Report Summary");

    QObject::connect(ok_btn, SIGNAL(clicked(bool)), this, SLOT(addCommentGenerate()));

    m_commentReport->show();
}


void Reporter::addCommentGenerate()
{
    m_comment = m_txt->toPlainText();
    m_commentReport->close();
    this->generateReport();
}


void Reporter::generateReport()
{
    int c_testcase = 0;
    int c_success = 0;
    int c_fail = 0;
    int c_untested = 0;

    float rate_success, rate_fail, rate_untested;

    int node_vec_sz = m_proj->node_data.size();
    for (int i = 0; i < node_vec_sz; i++)
    {
        if (!m_proj->node_data[i].isTestCase) {
            continue;
        }

        c_testcase += 1;
        if (m_proj->node_data[i].testResult == "Pass") {
            c_success += 1;
        }
        if (m_proj->node_data[i].testResult == "Fail") {
            c_fail += 1;
        }
    }

    c_untested = c_testcase - c_success - c_fail;
    rate_success = float(c_success) / float(c_testcase) * 100;
    rate_fail = float(c_fail) / float(c_testcase) * 100;
    rate_untested = 100 - rate_success - rate_fail;

    // set precision
    std::stringstream ss_rate_success, ss_rate_fail, ss_rate_untested;
    ss_rate_success << std::fixed << std::setprecision(2) << rate_success;
    ss_rate_fail << std::fixed << std::setprecision(2) << rate_fail;
    ss_rate_untested << std::fixed << std::setprecision(2) << rate_untested;

    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream curr_date_time;
    curr_date_time << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");

    std::string html  = std::string("<html>") +\
                       std::string("<header><style>table,th,td{border: 1px solid black; border-collapse:collapse; padding: 2px} hr{height:0px;background-color:#000;border:none;}div{white-space: pre-wrap;}</style>")  +\
                       std::string("<h3>Test Summary Report</h3></header><hr/>") +\
                       std::string("<body><h4>App: QoolTester</h4>") +\
                       std::string("<h4>Date: ") + curr_date_time.str() + std::string("</h4>") +\
                       std::string("<h4>Success: <strong style=\"color:darkgreen;\">") + ss_rate_success.str() + "%</strong>   [ " + std::to_string(c_success) +"/" + std::to_string(c_testcase) + " ]" + std::string("</h4>") +\
                       std::string("<h4>Failed: <strong style=\"color:red;\">") + ss_rate_fail.str() + "%</strong>   [ " + std::to_string(c_fail) +"/" + std::to_string(c_testcase) + " ]" + std::string("</h4>") +\
                       std::string("<h4>Untested: ") + ss_rate_untested.str() + "%   [ " + std::to_string(c_untested) +"/" + std::to_string(c_testcase) + " ]" + std::string("</h4><hr/>");

    html +=  std::string("<div><h4>Introduction: </h4></div><div>") + m_comment.toStdString() + "</div><p><p><p><p>";

    html += std::string("<table style=\"width:100%\"><tr><th>Test case</th><th>Status</th></tr>");
    for (int i = 0; i < node_vec_sz; i++)
    {  
        html += std::string("<tr><td>") + "UID-" + m_proj->node_data[i].full_name + std::string("</td>");
        if (m_proj->node_data[i].isTestCase && m_proj->node_data[i].testResult == "Pass"){
            html += std::string("<td><strong style=\"color:darkgreen;\">PASSED</strong></td></tr>");
        }
        else if (m_proj->node_data[i].isTestCase && m_proj->node_data[i].testResult == "Fail"){
            html += std::string("<td><strong style=\"color:red;\">FAILED</strong></td></tr>");
        }
        else if (!m_proj->node_data[i].isTestCase){
            html += std::string("<td>-</td></tr>");
        }
        else {
            html += std::string("<td><b>Untested</b></td></tr>");
        }
    }
    html += std::string("</table></body></html>");

    QTextDocument document;
    document.setHtml(QString::fromStdString(html));
    QPrinter printer;
    printer.setPageMargins(QMarginsF(15, 15, 15, 15));
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.printRange();
    printer.setOutputFileName(QString::fromStdString(m_reportPath));
    document.print(&printer);

}
