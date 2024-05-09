#include "ImageViewer.h"
#include <iostream>


ImageViewer::ImageViewer(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::ImageViewerClass)
{
	ui->setupUi(this);
	vW = new ViewerWidget(QSize(500, 500));
	ui->scrollArea->setWidget(vW);

	ui->scrollArea->setBackgroundRole(QPalette::Dark);
	ui->scrollArea->setWidgetResizable(true);
	ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	vW->setObjectName("ViewerWidget");
	vW->installEventFilter(this);

	globalColor = Qt::blue;
	vW->setFillColor(Qt::darkBlue);
	vW->setFillColor2(Qt::darkRed);
	vW->setFillColor3(Qt::darkGreen);
	QString style_sheet = QString("background-color: #%1;").arg(globalColor.rgba(), 0, 16);
	QString style_sheet1 = QString("background-color: #%1;").arg(vW->fillColor_().rgba(), 0, 16);

	ui->pushButtonSetColor->setStyleSheet(style_sheet);
	ui->pushButtonSetColor1->setStyleSheet(style_sheet1);

	QString style_sheet_disabled = "background-color: #C0C0C0; color: #FFFFFF;"; // Adjust colors as needed
	ui->pushButtonSetColor2->setDisabled(true);
	ui->pushButtonSetColor2->setStyleSheet(style_sheet_disabled);
	ui->pushButtonSetColor2->setText("Not available");

	ui->pushButtonSetColor3->setDisabled(true);
	ui->pushButtonSetColor3->setStyleSheet(style_sheet_disabled);
	ui->pushButtonSetColor3->setText("Not available");

	ui->line->setChecked(true);
	connect(ui->line, &QRadioButton::toggled, this, &ImageViewer::handleRadioToggled);
	connect(ui->polygon, &QRadioButton::toggled, this, &ImageViewer::handleRadioToggled);


	connect(ui->posuvanie, &QRadioButton::toggled, this, &ImageViewer::posuvanieCheck);
	connect(ui->otacanie, &QRadioButton::toggled, this, &ImageViewer::otacanieCheck);
	connect(ui->skalovanie, &QRadioButton::toggled, this, &ImageViewer::skalovanieCheck);
	connect(ui->osovaSumernost, &QRadioButton::toggled, this, &ImageViewer::osovaSumernostCheck);
	connect(ui->skosenie, &QRadioButton::toggled, this, &ImageViewer::skosenieCheck);

	ui->typeOfFilling->setDisabled(true);

	vW->initHlbkaPixelov();

	ui->distance->setValue(300);
}


// Event filters
bool ImageViewer::eventFilter(QObject* obj, QEvent* event)
{
	if (obj->objectName() == "ViewerWidget") {
		return ViewerWidgetEventFilter(obj, event);
	}
	return false;
}

//ViewerWidget Events
bool ImageViewer::ViewerWidgetEventFilter(QObject* obj, QEvent* event)
{
	ViewerWidget* w = static_cast<ViewerWidget*>(obj);

	if (!w) {
		return false;
	}

	if (event->type() == QEvent::MouseButtonPress) {
		ViewerWidgetMouseButtonPress(w, event);
	}
	else if (event->type() == QEvent::MouseButtonRelease) {
		ViewerWidgetMouseButtonRelease(w, event);
	}
	else if (event->type() == QEvent::MouseMove) {
		ViewerWidgetMouseMove(w, event);
	}
	else if (event->type() == QEvent::Leave) {
		ViewerWidgetLeave(w, event);
	}
	else if (event->type() == QEvent::Enter) {
		ViewerWidgetEnter(w, event);
	}
	else if (event->type() == QEvent::Wheel) {
		ViewerWidgetWheel(w, event);
	}
	return QObject::eventFilter(obj, event);
}

void ImageViewer::ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
	QRadioButton* clickedButton = qobject_cast<QRadioButton*>(sender());

	//musi byt zapnuty radio button na line, nesmie byt line uz nakreslena a ma byt aktivny tab, kde sa kreslia line
	if (e->button() == Qt::LeftButton && ui->line->isChecked() && lineIsDone == false && ui->tabWidget->currentIndex() == 0)
	{
		//zadanie druheho bodu a vykreslenie ciary
		if (w->getDrawLineActivated())
		{
			w->drawLine(w->getDrawLineBegin(), e->pos(), globalColor, ui->comboBoxLineAlg->currentIndex());
			lineIsDone = true;
			w->addLinePoint(e->pos());
			ui->line->setDisabled(true);
			w->setDrawLineActivated(false);
		}
		//zadanie prveho bodu
		else
		{
			w->setDrawLineBegin(e->pos());
			w->addLinePoint(e->pos());
			w->setDrawLineActivated(true);
			w->setPixel(e->pos().x(), e->pos().y(), globalColor);
			w->update();
		}
	}
	//musi byt zapnuty radio button na circle a ma byt aktivny tab, kde sa kreslia line
	if (e->button() == Qt::LeftButton && ui->circle->isChecked() && ui->tabWidget->currentIndex() == 1)
	{
		if (w->getDrawLineActivated()) {
			w->drawCircle(w->getDrawLineBegin(), e->pos(), globalColor);
			w->setDrawLineActivated(false);
		}
		else {
			w->setDrawLineBegin(e->pos());
			w->setDrawLineActivated(true);
			w->setPixel(e->pos().x(), e->pos().y(), globalColor);
			w->update();
		}
	}
	//musi byt zapnuty radio button na elipsu a ma byt aktivny tab, kde sa kreslia elipsy
	if (e->button() == Qt::LeftButton && ui->ellipse->isChecked() && ui->tabWidget->currentIndex() == 1)
	{
		w->setPixel(e->pos().x(), e->pos().y(), globalColor);
		w->addEllipsePoint(e->pos());
		//musia byt zadane tri body - stred, x polomer, y polomer
		if (w->ellipseRadius().size() == 3)
		{
			w->drawEllipse(w->ellipseRadius()[0], w->ellipseRadius()[1], w->ellipseRadius()[2], globalColor);
			vW->clearEllipse();
		}
		w->update();
	}
	//musi byt zapnuty radio button na polygon, nesmie byt nakresleny ziadny iny polygon a ma byt aktivny tab, kde sa kreslia polygony
	if (e->button() == Qt::LeftButton && ui->polygon->isChecked() && polygonIsDone == false && ui->tabWidget->currentIndex() == 0)
	{
		w->setPixel(e->pos().x(), e->pos().y(), globalColor);
		w->addPolygonPoint(e->pos());
		w->update();
	}
	//zadavanie bodo pre polygon sa konci kliknutim praveho tlacidla
	if (e->button() == Qt::RightButton && ui->polygon->isChecked())
	{
		if (w->polygon_points().size() == 3)
		{
			ui->typeOfFilling->setDisabled(false);
			//tri farby su pre barycentric a closest vypln
			if (ui->typeOfFilling->currentIndex() != 2)
			{
				//nastavim tlacidlam farbu
				QString style_sheet2 = QString("background-color: #%1;").arg(vW->fillColor2_().rgba(), 0, 16);
				QString style_sheet3 = QString("background-color: #%1;").arg(vW->fillColor3_().rgba(), 0, 16);
				ui->pushButtonSetColor2->setStyleSheet(style_sheet2);
				ui->pushButtonSetColor3->setStyleSheet(style_sheet3);
				ui->pushButtonSetColor2->setDisabled(false);
				ui->pushButtonSetColor3->setDisabled(false);
				ui->pushButtonSetColor2->setText("");
				ui->pushButtonSetColor3->setText("");
			}
			//jedna farba pre solid color
			else
			{
				//nastavime tlacidla default farbu a text
				QString style_sheet_disabled = "background-color: #C0C0C0; color: #FFFFFF;"; // Adjust colors as needed
				ui->pushButtonSetColor2->setDisabled(true);
				ui->pushButtonSetColor2->setStyleSheet(style_sheet_disabled);
				ui->pushButtonSetColor2->setText("Not available");

				ui->pushButtonSetColor3->setDisabled(true);
				ui->pushButtonSetColor3->setStyleSheet(style_sheet_disabled);
				ui->pushButtonSetColor3->setText("Not available");
			}
		}
		//ak polygon nie je trojuholnik, tak sa nda vybrat typ vyplne
		else
		{
			ui->typeOfFilling->setDisabled(true);
		}
		//vyfarbenie - true alebo false
		if (ui->vyfarbenie->isChecked())
		{
			w->drawPolygon(w->polygon_points(), globalColor, ui->comboBoxLineAlg->currentIndex(), true);
		}
		else
		{
			w->drawPolygon(w->polygon_points(), globalColor, ui->comboBoxLineAlg->currentIndex(), false);
		}
		ui->polygon->setDisabled(true);
		polygonIsDone = true;

	}
	//musi byt zapnuty radio button na krivku, nesmie byt nakreslena ziadna ina krivka a ma byt aktivny tab, kde sa kreslia krivky
	if (e->button() == Qt::LeftButton && ui->drawCurve->isChecked() && curveIsDone == false && ui->tabWidget->currentIndex() == 1)
	{
		w->setPixel(e->pos().x(), e->pos().y(), globalColor);
		w->drawCircle(e->pos(), QPoint(e->pos().x() + 1, e->pos().y() + 1), globalColor);
		w->addCurvePoint(e->pos());
		//ak je typ krivky hermit, tak sa kreslia ak dotycnice
		if (ui->typeOfCurve->currentIndex() == 0)
		{
			w->addSmernica(0); //defaultne maju smernicu 1
			QString string = "x: " + QString::number(e->pos().x()) + " y: " + QString::number(e->pos().y());
			ui->bodyKrivky->addItem(string); //pridavame body do combo boxu
			vW->drawLineWithLengthAndSlope(e->pos(), 70, 0, globalColor, ui->comboBoxLineAlg->currentIndex()); //vykreslime dotycnice - defaultne dlzka 50px
		}
		w->update();
	}
}

void ImageViewer::ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
}

//posuvanie 
void ImageViewer::ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
	if (ui->polygon->isChecked())
	{
		if (e->buttons() & Qt::LeftButton && ui->posuvanie->isChecked())
		{
			QPoint offset = e->pos() - w->getMoveBegin();
			if (!w->getMoveBegin().isNull())
			{
				if (ui->vyfarbenie->isChecked())
				{
					w->posuvanie(w->polygon_points(), globalColor, offset, ui->comboBoxLineAlg->currentIndex(), true);
				}
				else
				{
					w->posuvanie(w->polygon_points(), globalColor, offset, ui->comboBoxLineAlg->currentIndex(), false);
				}

			}
			w->setMoveBegin(e->pos());
		}
		else if (ui->posuvanie->isChecked())
		{
			w->setMoveBegin(QPoint());
		}
	}
	else if (ui->line->isChecked())
	{
		if (e->buttons() & Qt::LeftButton && ui->posuvanie->isChecked())
		{
			QPoint offset = e->pos() - w->getMoveBegin();
			if (!w->getMoveBegin().isNull())
			{
				w->clear();
				w->setLinePoint(w->line_points()[0] + offset, 0);
				w->setLinePoint(w->line_points()[1] + offset, 1);
				w->drawLine(w->line_points()[0], w->line_points()[1], globalColor, ui->comboBoxLineAlg->currentIndex());
				if (ui->vyfarbenie->isChecked())
				{
					w->drawPolygon(w->polygon_points(), globalColor, ui->comboBoxLineAlg->currentIndex(), true);
				}
				else
				{
					w->drawPolygon(w->polygon_points(), globalColor, ui->comboBoxLineAlg->currentIndex(), false);
				}
				update();

			}
			w->setMoveBegin(e->pos());
		}
		else if (ui->posuvanie->isChecked())
		{
			w->setMoveBegin(QPoint());
		}
	}

}

void ImageViewer::ViewerWidgetLeave(ViewerWidget* w, QEvent* event)
{
}
void ImageViewer::ViewerWidgetEnter(ViewerWidget* w, QEvent* event)
{
}
//skalovanie
void ImageViewer::ViewerWidgetWheel(ViewerWidget* w, QEvent* event)
{
	if (ui->skalovanie->isChecked())
	{
		QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
		double scale = 1.0;

		int deltaY = wheelEvent->angleDelta().y();

		if (deltaY > 0) {
			scale = 1.25;
		}
		else if (deltaY < 0) {
			scale = 0.75;
		}
		if (ui->polygon->isChecked())
		{
			if (ui->vyfarbenie->isChecked())
			{
				w->skalovanie(vW->polygon_points(), globalColor, scale, scale, ui->comboBoxLineAlg->currentIndex(), true);
			}
			else
			{
				w->skalovanie(vW->polygon_points(), globalColor, scale, scale, ui->comboBoxLineAlg->currentIndex(), false);
			}
		}
		else
		{
			w->skalovanie(vW->line_points(), globalColor, scale, scale, ui->comboBoxLineAlg->currentIndex(), false);
		}
	}
}


//ImageViewer Events
void ImageViewer::closeEvent(QCloseEvent* event)
{
	if (QMessageBox::Yes == QMessageBox::question(this, "Close Confirmation", "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No))
	{
		event->accept();
	}
	else {
		event->ignore();
	}
}

//Image functions
bool ImageViewer::openImage(QString filename)
{
	QImage loadedImg(filename);
	if (!loadedImg.isNull()) {
		return vW->setImage(loadedImg);
	}
	return false;
}
bool ImageViewer::saveImage(QString filename)
{
	QFileInfo fi(filename);
	QString extension = fi.completeSuffix();

	QImage* img = vW->getImage();
	return img->save(filename, extension.toStdString().c_str());
}

//Slots
/*
void ImageViewer::on_actionOpen_triggered()
{
	QString folder = settings.value("folder_img_load_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm .*xbm .* xpm);;All files (*)";
	QString fileName = QFileDialog::getOpenFileName(this, "Load image", folder, fileFilter);
	if (fileName.isEmpty()) { return; }

	QFileInfo fi(fileName);
	settings.setValue("folder_img_load_path", fi.absoluteDir().absolutePath());

	if (!openImage(fileName)) {
		msgBox.setText("Unable to open image.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}
}
*/
void ImageViewer::on_actionOpen_triggered()
{
	QString folder = settings.value("folder_img_load_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm *vtk)";
	QString fileName = QFileDialog::getOpenFileName(this, "Load file", folder, fileFilter);
	if (fileName.isEmpty()) { return; }

	QFileInfo fi(fileName);
	settings.setValue("folder_img_load_path", fi.absoluteDir().absolutePath());

	if (fileName.endsWith(".vtk", Qt::CaseInsensitive)) {
		// Súbor s príponou .vtk bol vybraný
		vW->ulozKockuZoSuboru(fileName.toStdWString());
		printf("VYBRATY VTK SUBOR\n");
		if (ui->typPremietania->currentIndex() == 0)
			vW->pohladovaSustava(ui->zenit->value(), ui->azimut->value(), true, ui->distance->value());
		else
			vW->pohladovaSustava(ui->zenit->value(), ui->azimut->value(), false, ui->distance->value());

		if (ui->phongWant->isChecked())
		{
			nastavPreFarbu();
		}
		else
		{
			vW->premietanie(ui->vyfarbit->isChecked());
		}
	}
	else {
		// Iný súbor bol vybraný, voláme openImage() pre obrázky
		if (!openImage(fileName)) {
			msgBox.setText("Unable to open image.");
			msgBox.setIcon(QMessageBox::Warning);
			msgBox.exec();
		}
	}
}
void ImageViewer::on_actionSave_as_triggered()
{
	QString folder = settings.value("folder_img_save_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm .*xbm .* xpm);;All files (*)";
	QString fileName = QFileDialog::getSaveFileName(this, "Save image", folder, fileFilter);
	if (!fileName.isEmpty()) {
		QFileInfo fi(fileName);
		settings.setValue("folder_img_save_path", fi.absoluteDir().absolutePath());

		if (!saveImage(fileName)) {
			msgBox.setText("Unable to save image.");
			msgBox.setIcon(QMessageBox::Warning);
		}
		else {
			msgBox.setText(QString("File %1 saved.").arg(fileName));
			msgBox.setIcon(QMessageBox::Information);
		}
		msgBox.exec();
	}
}
//clear - vyclearovanie aj vectorov bodov
void ImageViewer::on_actionClear_triggered()
{
	vW->clear();

	ui->polygon->setDisabled(false);
	polygonIsDone = false;
	vW->clearPolygon();

	lineIsDone = false;
	ui->line->setDisabled(false);
	vW->clearLine();

	vW->clearCurve();

	vW->getMoveBegin() = QPoint();
	ui->bodyKrivky->clear();

	QString style_sheet = QString("background-color: #%1;").arg(globalColor.rgba(), 0, 16);
	QString style_sheet1 = QString("background-color: #%1;").arg(vW->fillColor_().rgba(), 0, 16);

	ui->pushButtonSetColor->setStyleSheet(style_sheet);
	ui->pushButtonSetColor1->setStyleSheet(style_sheet1);

	QString style_sheet_disabled = "background-color: #C0C0C0; color: #FFFFFF;"; // Adjust colors as needed
	ui->pushButtonSetColor2->setDisabled(true);
	ui->pushButtonSetColor2->setStyleSheet(style_sheet_disabled);
	ui->pushButtonSetColor2->setText("Not available");

	ui->pushButtonSetColor3->setDisabled(true);
	ui->pushButtonSetColor3->setStyleSheet(style_sheet_disabled);
	ui->pushButtonSetColor3->setText("Not available");

	vW->initHlbkaPixelov();
	vW->hranyKocky.clear();
	vW->novaKocka.clear();
	vW->vrcholy.clear();
	vW->vrcholy1.clear();
}

void ImageViewer::on_actionExit_triggered()
{
	this->close();
}

//nastavovanie farieb

void ImageViewer::on_pushButtonSetColor_clicked()
{
	QColor newColor = QColorDialog::getColor(globalColor, this);
	if (newColor.isValid()) {
		QString style_sheet = QString("background-color: #%1;").arg(newColor.rgba(), 0, 16);
		ui->pushButtonSetColor->setStyleSheet(style_sheet);
		globalColor = newColor;
		if (ui->polygon->isChecked())
		{
			if (vW->polygon_points().size() != 0)
				vW->drawPolygon(vW->polygon_points(), globalColor, ui->comboBoxLineAlg->currentIndex(), true);
		}
		else if (ui->line->isChecked())
		{
			if (vW->line_points().size() != 0)
				vW->drawLine(vW->line_points()[0], vW->line_points()[1], globalColor, ui->comboBoxLineAlg->currentIndex());
		}
	}
}

void ImageViewer::on_pushButtonSetColor1_clicked()
{
	QColor newColor = QColorDialog::getColor(vW->fillColor_(), this);
	if (newColor.isValid()) {
		QString style_sheet = QString("background-color: #%1;").arg(newColor.rgba(), 0, 16);
		ui->pushButtonSetColor1->setStyleSheet(style_sheet);
		vW->setFillColor(newColor);
		//fillColor = newColor;
		if (ui->polygon->isChecked())
		{
			if (vW->polygon_points().size() != 0)
			{
				if (ui->vyfarbenie->isChecked())
				{
					vW->drawPolygon(vW->polygon_points(), globalColor, ui->comboBoxLineAlg->currentIndex(), true);
				}
				else
				{
					vW->drawPolygon(vW->polygon_points(), globalColor, ui->comboBoxLineAlg->currentIndex(), false);
				}

			}
		}
		else if (ui->line->isChecked())
		{
			if (vW->line_points().size() != 0)
				vW->drawLine(vW->line_points()[0], vW->line_points()[1], globalColor, ui->comboBoxLineAlg->currentIndex());
		}
	}
}
void ImageViewer::on_pushButtonSetColor2_clicked()
{
	QColor newColor = QColorDialog::getColor(vW->fillColor2_(), this);
	if (newColor.isValid()) {
		QString style_sheet = QString("background-color: #%1;").arg(newColor.rgba(), 0, 16);
		ui->pushButtonSetColor2->setStyleSheet(style_sheet);
		vW->setFillColor2(newColor);
		//	fillColor2 = newColor;
		if (ui->polygon->isChecked())
		{
			if (vW->polygon_points().size() != 0)
				vW->drawPolygon(vW->polygon_points(), globalColor, ui->comboBoxLineAlg->currentIndex(), true);
		}
		else if (ui->line->isChecked())
		{
			if (vW->line_points().size() != 0)
				vW->drawLine(vW->line_points()[0], vW->line_points()[1], globalColor, ui->comboBoxLineAlg->currentIndex());
		}
	}
}
void ImageViewer::on_pushButtonSetColor3_clicked()
{
	QColor newColor = QColorDialog::getColor(vW->fillColor3_(), this);
	if (newColor.isValid()) {
		QString style_sheet = QString("background-color: #%1;").arg(newColor.rgba(), 0, 16);
		ui->pushButtonSetColor3->setStyleSheet(style_sheet);
		vW->setFillColor3(newColor);
		//	fillColor3 = newColor;
		if (ui->polygon->isChecked())
		{
			if (vW->polygon_points().size() != 0)
				vW->drawPolygon(vW->polygon_points(), globalColor, ui->comboBoxLineAlg->currentIndex(), true);
		}
		else if (ui->line->isChecked())
		{
			if (vW->line_points().size() != 0)
				vW->drawLine(vW->line_points()[0], vW->line_points()[1], globalColor, ui->comboBoxLineAlg->currentIndex());
		}
	}
}

//otacanie
void ImageViewer::on_otocit_clicked()
{
	if (ui->otacanie->isChecked())
	{
		if (ui->polygon->isChecked())
		{
			if (ui->vyfarbenie->isChecked())
			{
				vW->otacanie(vW->polygon_points(), globalColor, ui->uholOtacania->value(), ui->comboBoxLineAlg->currentIndex(), true);
			}
			else
			{
				vW->otacanie(vW->polygon_points(), globalColor, ui->uholOtacania->value(), ui->comboBoxLineAlg->currentIndex(), false);
			}

		}
		else if (ui->line->isChecked())
		{
			vW->otacanie(vW->line_points(), globalColor, ui->uholOtacania->value(), ui->comboBoxLineAlg->currentIndex(), false);
		}
	}
}

//skalovanie
void ImageViewer::on_skalovat_clicked()
{
	if (ui->skalovanie->isChecked())
	{
		if (ui->polygon->isChecked())
		{
			if (ui->vyfarbenie->isChecked())
			{
				vW->skalovanie(vW->polygon_points(), globalColor, ui->skalovanieX->value(), ui->skalovanieY->value(), ui->comboBoxLineAlg->currentIndex(), true);
			}
			else
			{
				vW->skalovanie(vW->polygon_points(), globalColor, ui->skalovanieX->value(), ui->skalovanieY->value(), ui->comboBoxLineAlg->currentIndex(), false);
			}

		}
		else if (ui->line->isChecked())
		{
			vW->skalovanie(vW->line_points(), globalColor, ui->skalovanieX->value(), ui->skalovanieY->value(), ui->comboBoxLineAlg->currentIndex(), false);
		}

	}
}

//osovva sumernost
void ImageViewer::on_osovaSumernost_toggled()
{
	if (ui->osovaSumernost->isChecked())
	{
		if (ui->polygon->isChecked())
		{
			if (ui->vyfarbenie->isChecked())
			{
				vW->osova_sumernost(vW->polygon_points(), globalColor, ui->comboBoxLineAlg->currentIndex(), true);
			}
			else
			{
				vW->osova_sumernost(vW->polygon_points(), globalColor, ui->comboBoxLineAlg->currentIndex(), false);
			}
		}
		else if (ui->line->isChecked())
		{
			vW->osova_sumernost(vW->line_points(), globalColor, ui->comboBoxLineAlg->currentIndex(), false);
		}
	}
}

//skosenie
void ImageViewer::on_skosit_clicked()
{
	if (ui->skosenie->isChecked())
	{
		if (ui->polygon->isChecked())
		{
			if (ui->vyfarbenie->isChecked())
			{
				vW->skosenie(vW->polygon_points(), globalColor, ui->koeficientSkosenia->value(), ui->comboBoxLineAlg->currentIndex(), true);
			}
			else
			{
				vW->skosenie(vW->polygon_points(), globalColor, ui->koeficientSkosenia->value(), ui->comboBoxLineAlg->currentIndex(), false);
			}
		}
		else if (ui->line->isChecked())
		{
			vW->skosenie(vW->line_points(), globalColor, ui->koeficientSkosenia->value(), ui->comboBoxLineAlg->currentIndex(), false);
		}

	}
}

//vyfarbenie
void ImageViewer::on_vyfarbenie_toggled()
{
	if (ui->vyfarbenie->isChecked())
	{
		vW->clear();
		//vW->drawLine(vW->line_points()[0], vW->line_points()[1], globalColor, ui->comboBoxLineAlg->currentIndex());
		vW->drawPolygon(vW->polygon_points(), globalColor, ui->comboBoxLineAlg->currentIndex(), true);
	}
	else
	{
		vW->clear();
		//vW->drawLine(vW->line_points()[0], vW->line_points()[1], globalColor, ui->comboBoxLineAlg->currentIndex());
		vW->drawPolygon(vW->polygon_points(), globalColor, ui->comboBoxLineAlg->currentIndex(), false);
	}
}


//kreslenie krivky
void ImageViewer::on_typeOfCurve_currentIndexChanged()
{
	if (ui->typeOfCurve->currentIndex() != 0)
	{
		ui->zmenitSmernicu->setDisabled(true);
		ui->hodnotaSmernice->setDisabled(true);
	}
	else
	{
		ui->zmenitSmernicu->setDisabled(false);
		ui->hodnotaSmernice->setDisabled(false);
	}
	vW->set_typeOfFilling(ui->typeOfFilling->currentIndex());
}


//zmena typu vyplne v comboboxe
void ImageViewer::on_typeOfFilling_currentIndexChanged()
{
	if (ui->typeOfFilling->currentIndex() == 2)
	{
		QString style_sheet_disabled = "background-color: #C0C0C0; color: #FFFFFF;"; // Adjust colors as needed
		ui->pushButtonSetColor2->setDisabled(true);
		ui->pushButtonSetColor2->setStyleSheet(style_sheet_disabled);
		ui->pushButtonSetColor2->setText("Not available");

		ui->pushButtonSetColor3->setDisabled(true);
		ui->pushButtonSetColor3->setStyleSheet(style_sheet_disabled);
		ui->pushButtonSetColor3->setText("Not available");
	}
	if (ui->typeOfFilling->currentIndex() != 2)
	{
		QString style_sheet2 = QString("background-color: #%1;").arg(vW->fillColor2_().rgba(), 0, 16);
		QString style_sheet3 = QString("background-color: #%1;").arg(vW->fillColor3_().rgba(), 0, 16);
		ui->pushButtonSetColor2->setStyleSheet(style_sheet2);
		ui->pushButtonSetColor3->setStyleSheet(style_sheet3);
		ui->pushButtonSetColor2->setDisabled(false);
		ui->pushButtonSetColor3->setDisabled(false);
		ui->pushButtonSetColor2->setText("");
		ui->pushButtonSetColor3->setText("");
	}
	vW->set_typeOfFilling(ui->typeOfFilling->currentIndex());
	if (ui->vyfarbenie->isChecked())
	{
		vW->drawPolygon(vW->polygon_points(), globalColor, ui->comboBoxLineAlg->currentIndex(), true);
	}
}


//kreslenie krivky az po kliknuti tlacidla
void ImageViewer::on_drawCurveButton_clicked()
{

	if (ui->typeOfCurve->currentIndex() == 0)
	{
		if (vW->curvePoints().size() > 1)
		{
			vW->hermit(Qt::red, ui->comboBoxLineAlg->currentIndex());
		}
		else
		{
			QMessageBox::warning(this, "Chyba", "Pre tento typ krivky musite mat minimalne 2 body.");
		}
	}
	else if (ui->typeOfCurve->currentIndex() == 1)
	{
		if (vW->curvePoints().size() > 1)
		{
			vW->bezier(Qt::red, ui->comboBoxLineAlg->currentIndex());
		}
		else
		{
			QMessageBox::warning(this, "Chyba", "Pre tento typ krivky musite mat minimalne 2 body.");
		}
	}
	else if (ui->typeOfCurve->currentIndex() == 2)
	{
		if (vW->curvePoints().size() > 3)
		{
			vW->coonsov(Qt::red, ui->comboBoxLineAlg->currentIndex());
		}
		else
		{
			QMessageBox::warning(this, "Chyba", "Pre tento typ krivky musite mat minimalne 4 body.");
		}
	}
}

//prestavovanie smernic
void ImageViewer::on_bodyKrivky_currentIndexChanged()
{
	for (int i = 0; i < vW->curvePoints().size(); i++)
	{
		if (i == ui->bodyKrivky->currentIndex())
		{
			ui->hodnotaSmernice->setValue(vW->getSmernica(i));
		}
	}
}


//menenie smernice
void ImageViewer::on_zmenitSmernicu_clicked()
{
	vW->setSmernica(ui->hodnotaSmernice->value(), ui->bodyKrivky->currentIndex());

	vW->clear();
	for (int i = 0; i < vW->curvePoints().size(); i++)
	{
		vW->drawCircle(vW->curvePoints()[i], QPoint(vW->curvePoints()[i].x() + 1, vW->curvePoints()[i].y() + 1), globalColor);
		vW->drawLineWithLengthAndSlope(vW->curvePoints()[i], 70, vW->smernice()[i], globalColor, ui->comboBoxLineAlg->currentIndex());
	}
}


void ImageViewer::on_azimut_valueChanged()
{
	vW->originalLightSource = { (float)(ui->zdroj_x->value()),(float)(ui->zdroj_y->value()),(float)(ui->zdroj_z->value()) };
	ui->azimutValue->setText(QString::number(ui->azimut->value()));
	if (ui->typPremietania->currentIndex() == 0) //stredove
	{
		vW->clear();
		vW->pohladovaSustava(ui->zenit->value(), ui->azimut->value(), true, ui->distance->value());
		if (ui->phongWant->isChecked())
		{
			nastavPreFarbu();
		}
		else
		{
			vW->premietanie(ui->vyfarbit->isChecked());
		}
	}
	else //rovnobezne
	{
		double zenit = ui->zenit->value();
		double azimut = ui->azimut->value();
		vW->clear();
		vW->pohladovaSustava(zenit * 1.0, azimut * 1.0, false, ui->distance->value());
		if (ui->phongWant->isChecked())
		{
			nastavPreFarbu();
		}
		else
		{
			vW->premietanie(ui->vyfarbit->isChecked());
		}
	}

}

void ImageViewer::on_zenit_valueChanged()
{
	vW->originalLightSource = { (float)(ui->zdroj_x->value()),(float)(ui->zdroj_y->value()),(float)(ui->zdroj_z->value()) };
	ui->zenitValue->setText(QString::number(ui->zenit->value()));
	if (ui->typPremietania->currentIndex() == 0) //stredove
	{
		vW->clear();
		vW->pohladovaSustava(ui->zenit->value(), ui->azimut->value(), true, ui->distance->value());
		if (ui->phongWant->isChecked())
		{
			nastavPreFarbu();
		}
		else
		{
			vW->premietanie(ui->vyfarbit->isChecked());
		}
	}
	else //rovnobezne
	{
		double zenit = ui->zenit->value();
		double azimut = ui->azimut->value();
		vW->clear();
		vW->pohladovaSustava(zenit * 1.0, azimut * 1.0, false, ui->distance->value());
		if (ui->phongWant->isChecked())
		{
			nastavPreFarbu();
		}
		else
		{
			vW->premietanie(ui->vyfarbit->isChecked());
		}
	}

}

void ImageViewer::on_drawSphere_clicked()
{
	vW->clear();
	nastavPreFarbu();
}

void ImageViewer::nastavPreFarbu()
{
	//svetlo
	vW->zdrojSvetla = { (float)(ui->zdroj_x->value()),(float)(ui->zdroj_y->value()),(float)(ui->zdroj_z->value()) };
	vW->originalLightSource = { (float)(ui->zdroj_x->value()),(float)(ui->zdroj_y->value()),(float)(ui->zdroj_z->value()) };
	vW->farbaLuca = { (float)(ui->red->value()), (float)(ui->green->value()), (float)(ui->blue->value()) };

	//odraz
	vW->odraz = { (float)(ui->odraz_x->value()),(float)(ui->odraz_y->value()) ,(float)(ui->odraz_z->value()) };
	vW->poziciaKamery = (float)(ui->kamera_z->value());
	vW->ostrost = ui->ostrost_odrazu->value();

	//difuzia
	vW->difuzia = { (float)(ui->difuzia_x->value()),(float)(ui->difuzia_y->value()),(float)(ui->difuzia_z->value()) };

	//ambient
	vW->farbySceny = { (float)(ui->red_2->value()), (float)(ui->green_2->value()), (float)(ui->blue_2->value()) };
	vW->ambient = { (float)(ui->ambient_x->value()),(float)(ui->ambient_y->value()),(float)(ui->ambient_z->value()) };

	vW->phongModel(true, ui->typeOffFill->currentIndex(), ui->vyfarbit->isChecked(), ui->tocitSvetlom->isChecked());
}