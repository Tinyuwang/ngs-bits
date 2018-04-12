#ifndef MULTISAMPLEDIALOG_H
#define MULTISAMPLEDIALOG_H

#include <QDialog>
#include "ui_MultiSampleDialog.h"
#include "SingleSampleAnalysisDialog.h"

//Dialog for starting a multi-sample analysis.
class MultiSampleDialog
	: public QDialog
{
	Q_OBJECT

public:
	MultiSampleDialog(QWidget* parent = 0);
	//Fills table with given processed samples
	void setSamples(QList<AnalysisJobSample> samples);

	//Returns the processed sample list.
	QList<AnalysisJobSample> samples() const;
	//Returns the command line arguments.
	QStringList arguments() const;

private slots:
	void on_add_control_clicked(bool);
	void on_add_affected_clicked(bool);
	void on_clear_clicked(bool);
	void updateStartButton();

private:
	Ui::MultiSampleDialog ui_;
	NGSD db_;
	QList<SampleDetails> samples_;
	QList<AnalysisStep> steps_;

	void addSample(QString status, QString sample="");
	void updateSampleTable();
};

#endif // MULTISAMPLEDIALOG_H
