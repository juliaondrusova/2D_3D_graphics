#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets>
#include "ui_ImageViewer.h"
#include "ViewerWidget.h"


class ImageViewer : public QMainWindow
{
	Q_OBJECT

public:
	ImageViewer(QWidget* parent = Q_NULLPTR);

	void nastavPreFarbu();

private:
	Ui::ImageViewerClass* ui;
	ViewerWidget* vW;

	QColor globalColor;
	QSettings settings;
	QMessageBox msgBox;

	bool polygonIsDone = false;
	bool lineIsDone = false;
	bool ellipseIsDone = false;
	bool curveIsDone = false;
	//Event filters
	bool eventFilter(QObject* obj, QEvent* event);

	//ViewerWidget Events
	bool ViewerWidgetEventFilter(QObject* obj, QEvent* event);
	void ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event);
	void ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event);
	void ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event);
	void ViewerWidgetLeave(ViewerWidget* w, QEvent* event);
	void ViewerWidgetEnter(ViewerWidget* w, QEvent* event);
	void ViewerWidgetWheel(ViewerWidget* w, QEvent* event);

	//ImageViewer Events
	void closeEvent(QCloseEvent* event);

	//Image functions
	bool openImage(QString filename);
	bool saveImage(QString filename);

private slots:
	void on_actionOpen_triggered();
	void on_actionSave_as_triggered();
	void on_actionClear_triggered();
	void on_actionExit_triggered();

	//Tools slots
	void on_azimut_valueChanged();
	void on_zenit_valueChanged();

	void on_drawSphere_clicked();

	void on_pushButtonSetColor_clicked();
	void on_pushButtonSetColor1_clicked();
	void on_pushButtonSetColor2_clicked();
	void on_pushButtonSetColor3_clicked();
	void on_otocit_clicked();
	void on_skalovat_clicked();
	void on_osovaSumernost_toggled();
	void on_skosit_clicked();
	void on_vyfarbenie_toggled();
	void on_typeOfFilling_currentIndexChanged();
	void on_bodyKrivky_currentIndexChanged();
	void on_typeOfCurve_currentIndexChanged(); 
	void on_drawCurveButton_clicked();
	void on_zmenitSmernicu_clicked();
	void handleRadioToggled(bool checked)
	{
		if (!checked)
		{
			ui->comboBoxLineAlg->setEnabled(false);
		}
		else
		{
			ui->comboBoxLineAlg->setEnabled(true);
		}
	}
	void setCheckboxesDisabled(QCheckBox* currentCheckbox)
	{
		QVector<QCheckBox*> checkboxes = { ui->otacanie, ui->skalovanie, ui->osovaSumernost, ui->skosenie, ui->posuvanie};
		for (QCheckBox* checkbox : checkboxes)
		{

			if (checkbox != currentCheckbox)
			{
				checkbox->setDisabled(currentCheckbox->isChecked());
				checkbox->setChecked(false);
			}
			else
			{
				checkbox->setDisabled(false);
			}
		}
	}

	void posuvanieCheck() {
		setCheckboxesDisabled(ui->posuvanie);
	}
	void otacanieCheck() {
		setCheckboxesDisabled(ui->otacanie);
	}

	void skalovanieCheck() {
		setCheckboxesDisabled(ui->skalovanie);
	}
	void osovaSumernostCheck() {
		setCheckboxesDisabled(ui->osovaSumernost);
	}
	void skosenieCheck() {
		setCheckboxesDisabled(ui->skosenie);
	}

};