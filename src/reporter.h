#pragma once

#include "project.h"
#include <QtWidgets>


class Reporter : public QWidget
{
    Q_OBJECT
public:
    Reporter(Project_data_t *proj, std::string reportPath);

    void generateReport();

public slots:
    void addCommentGenerate();

private:
    Project_data_t * m_proj;
    std::string m_reportPath;
    QDialog * m_commentReport;
    QPlainTextEdit * m_txt;
    QString m_comment;

};
