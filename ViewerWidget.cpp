#include "ViewerWidget.h"
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <QVector3D>


ViewerWidget::ViewerWidget(QSize imgSize, QWidget* parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_StaticContents);
	setMouseTracking(true);
	if (imgSize != QSize(0, 0)) {
		img = new QImage(imgSize, QImage::Format_ARGB32);
		img->fill(Qt::white);
		resizeWidget(img->size());
		setPainter();
		setDataPtr();
	}
}
ViewerWidget::~ViewerWidget()
{
	delete painter;
	delete img;
}
void ViewerWidget::resizeWidget(QSize size)
{
	this->resize(size);
	this->setMinimumSize(size);
	this->setMaximumSize(size);
}

//Image functions
bool ViewerWidget::setImage(const QImage& inputImg)
{
	if (img != nullptr) {
		delete painter;
		delete img;
	}
	img = new QImage(inputImg);
	if (!img) {
		return false;
	}
	resizeWidget(img->size());
	setPainter();
	setDataPtr();
	update();

	return true;
}
bool ViewerWidget::isEmpty()
{
	if (img == nullptr) {
		return true;
	}

	if (img->size() == QSize(0, 0)) {
		return true;
	}
	return false;
}

bool ViewerWidget::changeSize(int width, int height)
{
	QSize newSize(width, height);

	if (newSize != QSize(0, 0)) {
		if (img != nullptr) {
			delete painter;
			delete img;
		}

		img = new QImage(newSize, QImage::Format_ARGB32);
		if (!img) {
			return false;
		}
		img->fill(Qt::white);
		resizeWidget(img->size());
		setPainter();
		setDataPtr();
		update();
	}

	return true;
}

void ViewerWidget::setPixel(int x, int y, uchar r, uchar g, uchar b, uchar a)
{
	r = r > 255 ? 255 : (r < 0 ? 0 : r);
	g = g > 255 ? 255 : (g < 0 ? 0 : g);
	b = b > 255 ? 255 : (b < 0 ? 0 : b);
	a = a > 255 ? 255 : (a < 0 ? 0 : a);

	size_t startbyte = y * img->bytesPerLine() + x * 4;
	data[startbyte] = b;
	data[startbyte + 1] = g;
	data[startbyte + 2] = r;
	data[startbyte + 3] = a;
}
void ViewerWidget::setPixel(int x, int y, double valR, double valG, double valB, double valA)
{
	valR = valR > 1 ? 1 : (valR < 0 ? 0 : valR);
	valG = valG > 1 ? 1 : (valG < 0 ? 0 : valG);
	valB = valB > 1 ? 1 : (valB < 0 ? 0 : valB);
	valA = valA > 1 ? 1 : (valA < 0 ? 0 : valA);

	size_t startbyte = y * img->bytesPerLine() + x * 4;
	data[startbyte] = static_cast<uchar>(255 * valB);
	data[startbyte + 1] = static_cast<uchar>(255 * valG);
	data[startbyte + 2] = static_cast<uchar>(255 * valR);
	data[startbyte + 3] = static_cast<uchar>(255 * valA);
}
void ViewerWidget::setPixel(int x, int y, const QColor& color)
{
	if (color.isValid()) {
		size_t startbyte = y * img->bytesPerLine() + x * 4;

		data[startbyte] = color.blue();
		data[startbyte + 1] = color.green();
		data[startbyte + 2] = color.red();
		data[startbyte + 3] = color.alpha();
	}
}
void ViewerWidget::setPixel3D(int x, int y, const QColor& color, double zValue)
{
	
	if (y >= 500 || y < 0 || x >= 500 || x < 0)
		return;

	
	if (zValue <= hlbkaPixelov[x][y])
	{
		return;
	}

	else
	{
		hlbkaPixelov[x][y] = zValue;
		
		if (color.isValid()) {
			size_t startbyte = y * img->bytesPerLine() + x * 4;

			data[startbyte] = color.blue();
			data[startbyte + 1] = color.green();
			data[startbyte + 2] = color.red();
			data[startbyte + 3] = color.alpha();
		}
	}

}


void ViewerWidget::drawLineWithLengthAndSlope(QPoint startPoint, int length, double slope, QColor color, int algType)
{
	int endX, endY;

	if (slope == 0) {
		endX = startPoint.x() + length;
		endY = startPoint.y();
	}
	else {
		// Calculate the end point based on the given slope and the required length
		double angle = atan(slope);
		endX = startPoint.x() + static_cast<int>(length * cos(angle));
		endY = startPoint.y() + static_cast<int>(length * sin(angle));
	}

	drawLine(startPoint, QPoint(endX, endY), color, algType);
}


//kreslenie ciary
void ViewerWidget::drawLine(QPoint start, QPoint end, QColor color, int algType)
{
	double dx = end.x() - start.x();
	double dy = end.y() - start.y();


	//najprv treba polygon orezat - ak netreba, tak bude vrateny povodny
	std::vector<QPoint> line = clipLineByPolygon({ start, end }, color);

	if (line.size() == 0)
	{
		return;
	}
	QPoint newStart = line[0];
	QPoint newEnd = line[1];

	//DDA algoritmus
	if (algType == 0)
	{
		// kontrola, Ëi dx je vacsie ako dy - ciara je viac vodorovna ako zvisla (sklon je < 45)
		if (qAbs(dx) > qAbs(dy)) // smer x
		{
			// vymen koncovy a zaciatocny bod, ak ma zaciatocny bod vacsie suradnice
			if (newStart.x() > newEnd.x()) {
				std::swap(newStart, newEnd);
			}
			// Ak je dx 0 - nastavime ho na DBL_MAX, aby sme predisli deleniu 0
			if (dx == 0)
			{
				dx = DBL_MAX;
			}
			//vypocitame smernicu ciary
			double m = dy / dx;
			// Inicializujeme aktualnu y-ovu suradnicu na y-ovu suradnicu prveho bodu
			double actualY = newStart.y();
			// Iterujeme cez x-ove suradnice od start.x() po end.x()
			for (int i = newStart.x(); i <= newEnd.x(); i++)
			{
				// Nastavime farbu pera pre kreslenie
				painter->setPen(QPen(color));
				// vykreslime dany body bod - zaokruhlime y-ovu suradnicu
				painter->drawPoint(i, qRound(actualY));
				// zvacsime aktualnu y-ovu s˙radnicu o sklon pre dalsiu iteraciu
				actualY += m;
			}
		}
		else // smer y (sklon je > 45)
		{
			// vymen koncovy a zaciatocny bod, ak ma zaciatocny bod vacsie suradnice
			if (newStart.y() > newEnd.y()) {
				std::swap(newStart, newEnd);
			}
			// Ak je dy 0 - nastavime ho na DBL_MAX, aby sme predisli deleniu 0
			if (dy == 0)
			{
				dy = DBL_MAX;
			}
			//vypocitame smernicu ciary
			double m = dx / dy;
			// Inicializujeme aktualnu x-ovu suradnicu na x-ovu suradnicu prveho bodu
			double actualX = newStart.x();
			// Iterujeme cez y-ove suradnice od start.y() po end.y()
			for (int i = newStart.y(); i <= newEnd.y(); i++)
			{
				// Nastavime farbu pera pre kreslenie
				painter->setPen(QPen(color));
				// vykreslime dany body bod - zaokruhlime x-ovu suradnicu
				painter->drawPoint(qRound(actualX), i);
				// Inkrementujeme aktualnu x-ov˙ suradnicu o sklon pre dalsiu iteraciu
				actualX += m;
			}
		}
	}
	// Bresenhamov algoritmus
	else if (algType == 1)
	{
		// Urcenie kroku v smere osi X a Y podla znamienok rozdielu dx a dy
		int stepX = (dx > 0) ? 1 : -1;
		int stepY = (dy > 0) ? 1 : -1;

		dx = abs(dx);
		dy = abs(dy);

		//vykreslenie zaciatocneho bodu
		painter->drawPoint(start);

		if (dx > dy) // smer osi x
		{
			//inicializacia hodnoty rozhodovacieho clena
			int p = 2 * dy - dx;
			int y = start.y();

			// Iteracia cez os X od zaciatocneho bodu k cielovemu bodu
			for (int x = start.x() + stepX; x != end.x() + stepX; x += stepX)
			{
				painter->setPen(QPen(color));

				// Ak je hodnota parametra p vacsia alebo rovna 0
				if (p >= 0)
				{
					// Posun na dalsi bod v smere osi Y
					y += stepY;
					// Zniûznie hodnoty parametra p o dvojnasobok dx
					p -= 2 * dx;
				}

				// Zvysenie hodnoty parametra p o dvojnasobok dy
				p += 2 * dy;

				// Vykreslenie bodu
				painter->drawPoint(x, y);
			}
		}
		else // smer osi y
		{
			// inicializacia hodnoty parametra p
			int p = 2 * dx - dy;
			int x = start.x();

			// Iteracia cez os Y od zaciatocneho bodu k cielovemu bodu
			for (int y = start.y() + stepY; y != end.y() + stepY; y += stepY)
			{
				painter->setPen(QPen(color));

				// Ak je hodnota parametra p vacsia alebo rovna 0
				if (p >= 0)
				{
					// Posun na dalsi bod v smere osi X
					x += stepX;
					// znizenie hodnoty parametra p o dvojnasobok dy
					p -= 2 * dy;
				}

				// zvysenie hodnoty parametra p o dvojnasobok dx
				p += 2 * dx;

				// Vykreslenie bodu
				painter->drawPoint(x, y);
			}
		}
	}
	update();
}

void ViewerWidget::clear()
{
	img->fill(Qt::white);
	update();
}

//Slots
void ViewerWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	QRect area = event->rect();
	painter.drawImage(area, *img, area);
}

//kreslenie kruhu
void ViewerWidget::drawCircle(QPoint start, QPoint end, QColor color)
{
	//vypocitanie polomeru
	int r = sqrt(pow((end.x() - start.x()), 2) + pow((end.y() - start.y()), 2));
	int x = 0;
	int y = r;

	int p = 1 - r;
	int dvaX = 3;
	int dvaY = 2 * r - 2;

	painter->setPen(QPen(color));

	while (x <= y)
	{
		//vykreslenie symetrickych bodov
		painter->drawPoint(start.x() + x, start.y() + y);
		painter->drawPoint(start.x() - x, start.y() + y);
		painter->drawPoint(start.x() + x, start.y() - y);
		painter->drawPoint(start.x() - x, start.y() - y);
		painter->drawPoint(start.x() + y, start.y() + x);
		painter->drawPoint(start.x() - y, start.y() + x);
		painter->drawPoint(start.x() + y, start.y() - x);
		painter->drawPoint(start.x() - y, start.y() - x);

		if (p > 0)
		{
			p = p - dvaY;
			y--;
			dvaY = dvaY - 2;
		}
		p = p + dvaX;
		dvaX = dvaX + 2;
		x++;
	}
	update();
}

//kreslenie elipsy
void ViewerWidget::drawEllipse(QPoint center, QPoint point1, QPoint point2, QColor color)
{
	// Vypocitat vzdialenosti bodov od stredu elipsy
	int dist1X = abs(point1.x() - center.x());
	int dist1Y = abs(point1.y() - center.y());
	int dist2X = abs(point2.x() - center.x());
	int dist2Y = abs(point2.y() - center.y());

	int radiusX = abs(point1.x() - center.x());
	int radiusY = abs(point2.y() - center.y());

	// iicializovaù premenne pre algoritmus kreslenia elipsy
	int x = 0;
	int y = radiusY;

	int radiusXSquare = radiusX * radiusX;
	int radiusYSquare = radiusY * radiusY;
	int twoRadiusXSquare = 2 * radiusXSquare;
	int twoRadiusYSquare = 2 * radiusYSquare;

	int p = radiusYSquare - radiusXSquare * radiusY + 0.25 * radiusXSquare;

	painter->setPen(QPen(color));

	// kreslit elipsu
	while (twoRadiusYSquare * x < twoRadiusXSquare * y)
	{
		drawSymmetricPoints(center.x(), center.y(), x, y);
		if (p < 0)
		{
			x++;
			p += twoRadiusYSquare * x + radiusYSquare;
		}
		else
		{
			x++;
			y--;
			p += twoRadiusYSquare * x - twoRadiusXSquare * y + radiusYSquare;
		}
	}

	p = radiusYSquare * (x + 0.5) * (x + 0.5) + radiusXSquare * (y - 1) * (y - 1) - radiusXSquare * radiusYSquare;

	while (y >= 0)
	{
		drawSymmetricPoints(center.x(), center.y(), x, y);
		if (p > 0)
		{
			y--;
			p += radiusXSquare - twoRadiusXSquare * y;
		}
		else
		{
			y--;
			x++;
			p += twoRadiusYSquare * x - twoRadiusXSquare * y + radiusXSquare;
		}
	}
	update();
}


//kreslenie symetrcikych bodov - vyuzitie pri elipse
void ViewerWidget::drawSymmetricPoints(int cx, int cy, int x, int y)
{
	painter->drawPoint(cx + x, cy + y);
	painter->drawPoint(cx - x, cy + y);
	painter->drawPoint(cx + x, cy - y);
	painter->drawPoint(cx - x, cy - y);
}


//kreslenie polygonu
void ViewerWidget::drawPolygon(std::vector<QPoint> points, QColor color, int algType, bool vyfarbit, QColor fillColor)
{
	if (points.size() < 2) {
		return;
	}
	std::vector<QPoint> polygon = points;

	//najprv treba polygon orezat - ak netreba, tak bude vrateny povodny
	polygon = orezavanie(points, color);
	//ak orezany polygon je trojohulnik, tak newTriangle nastavime na true
	if (polygon.size() == 3)
	{
		newTrianglePoints = polygon;
		newTriangle = true;
	}
	//inak nastavime na false
	else
	{
		newTriangle = false;
	}
	//ak orezany polygonu ma velkost 0, tak dalej nic nerobime
	if (polygon.size() == 0)
	{
		return;
	}

	//vykreslime ciary medzi danymi bodmi
	for (size_t i = 0; i < polygon.size() - 1; i++)
	{
		drawLine(polygon.at(i), polygon.at(i + 1), color, algType);
	}
	//posledna ciara medzi poslednym a prvym bodom
	drawLine(polygon.back(), polygon.front(), color, algType);

	//ak je checknute vyfarbenie, tak ho vyfarbime
	if (vyfarbit == true)
	{
		//ak polygon nema 3 body, tak return - nie je to polygon
		if (polygon.size() < 3)
		{
			return;
		}
		//ak ma tri vrcholy, tak vyfarbujeme trojuholnik
		else if (polygon.size() == 3)
		{
			fillTriangle(polygon, fillColor, algType);
		}
		//inak vyfarbujeme klasicky polygon
		else
		{
			fillPolygon(polygon, fillColor);
		}
	}
}


//posuvanie
void ViewerWidget::posuvanie(std::vector<QPoint>& points, QColor color, QPoint offset, int algType, bool vyfarbit, QColor fillColor)
{
	//posunieme kazdy bod o dany offset
	for (size_t i = 0; i < points.size(); ++i) {
		points[i] += offset;
	}
	clear();
	drawPolygon(points, color, algType, vyfarbit, fillColor);
	update();
}


//otacanie
void ViewerWidget::otacanie(std::vector<QPoint>& points, QColor color, double angle, int algType, bool vyfarbit, QColor fillColor)
{
	//vypocitame uhol v radianoch
	double angleRadians = angle * M_PI / 180.0;

	//stred otacania nastavime na prvy bod 
	QPoint center = points.front();

	//pre kazdy bod v points
	for (size_t i = 0; i < points.size(); ++i)
	{

		//vypocitame relativne pozicie bodov od stredu
		double relativeX = points[i].x() - center.x();
		double relativeY = points[i].y() - center.y();

		//vypocitame nove suradnice - standartne cez sin a cos
		double newX = relativeX * cos(angleRadians) + relativeY * sin(angleRadians);
		double newY = -relativeX * sin(angleRadians) + relativeY * cos(angleRadians);

		//nastavime nove suradnice
		points[i].setX(center.x() + round(newX));
		points[i].setY(center.y() + round(newY));
	}
	clear();
	drawPolygon(line_points_, color, algType, false);
	drawPolygon(polygon_points_, color, algType, vyfarbit, fillColor);
	update();
}


//skalovanie
void ViewerWidget::skalovanie(std::vector<QPoint>& points, QColor color, double scaleX, double scaleY, int algType, bool vyfarbit, QColor fillColor)
{
	QPoint center = points.front();

	//ak su scale faktory zaporne, tak objekt sa ma aj preklopit
	if (scaleX < 0 || scaleY < 0) {
		osova_sumernost(points, color, algType, vyfarbit, fillColor);
	}

	//vypocitame nove suradnice bodov - kazdy bod naskalujeme
	for (QPoint& point : points)
	{
		double newX = center.x() + (point.x() - center.x()) * scaleX;
		double newY = center.y() + (point.y() - center.y()) * scaleY;

		point.setX(round(newX));
		point.setY(round(newY));
	}
	clear();
	drawPolygon(line_points_, color, algType, false);
	drawPolygon(polygon_points_, color, algType, vyfarbit, fillColor);
	update();

}

//osova sumernost
void ViewerWidget::osova_sumernost(std::vector<QPoint>& points, QColor color, int algType, bool vyfarbit, QColor fillColor)
{
	//osova sumernost usecky
	if (points.size() == 2)
	{
		int newX = points[1].x() + 2 * (points[0].x() - points[1].x());
		points[1].setX(newX);
	}
	//osova sumernost polygonu
	else {
		double A = points[1].y() - points[0].y();
		double B = points[0].x() - points[1].x();
		double C = -A * points[0].x() - B * points[0].y();
		for (QPoint& point : points) {
			double newX = point.x() - 2 * A * ((A * point.x() + B * point.y() + C) / (A * A + B * B));
			double newY = point.y() - 2 * B * ((A * point.x() + B * point.y() + C) / (A * A + B * B));
			point.setX(static_cast<int>(newX));
			point.setY(static_cast<int>(newY));
		}
	}

	clear();
	drawPolygon(line_points_, color, algType, false);
	drawPolygon(polygon_points_, color, algType, vyfarbit, fillColor);
	update();
}


//skosenie
void ViewerWidget::skosenie(std::vector<QPoint>& points, QColor color, double dx, int algType, bool vyfarbit, QColor fillColor)
{
	//skosenie len v smere x - vypocet novych suradnic
	for (int i = 1; i < points.size(); i++)
	{
		double noveX = points[i].x() + dx * points[i].y();
		points[i].setX(static_cast<int>(noveX));
	}

	clear();
	drawPolygon(line_points_, color, algType, false);
	drawPolygon(polygon_points_, color, algType, vyfarbit, fillColor);
	update();
}

//orezavanie
std::vector<QPoint> ViewerWidget::orezavanie(std::vector<QPoint> points, QColor color) {

	// Ak m· polygon menej ako 3 body, vr·ù pÙvodn˝ polygon, pretoûe nem· zmysel ho orezaù
	if (points.size() < 3)
	{
		return points;
	}
	std::vector<QPoint> W, V = points; // Inicializ·cia pomocn˝ch premenn˝ch
	QPoint S;

	// DefinÌcia obdÂûnika, do ktorÈho chceme orezaù polygon
	int xMin[] = { 5,5,-495,-495 };

	// Pre kaûd˙ stranu obdÂûnika
	for (int j = 0; j < 4; j++)
	{
		// Ak neexistuj˙ ûiadne body, vr·ù pr·zdny vektor bodov
		if (V.size() == 0)
		{
			return V;
		}
		S = V[V.size() - 1];  // NastavÌme bod S na posledn˝ bod polygonu

		// Pre kaûd˝ bod v polygone
		for (int i = 0; i < V.size(); i++)
		{
			if (V[i].x() >= xMin[j]) // Ak je bod vo vnutri  obdÂûnika
			{
				if (S.x() >= xMin[j]) // Ak aj predch·dzaj˙ci bod je vo vnutri  obdÂûnika
				{
					W.push_back(V[i]); // Pridaj bod do novÈho polygonu
				}
				else // Ak nie, tak vypoËÌtaj prienik s hranou obdÂûnika a pridaj novÈ body
				{
					QPoint P(xMin[j], S.y() + (xMin[j] - S.x()) * ((V[i].y() - S.y()) / (double)(V[i].x() - S.x())));
					W.push_back(P);
					W.push_back(V[i]);
				}
			}
			else // Ak bod nie je vo vnutri obdÂûnika
			{
				if (S.x() >= xMin[j]) // Ak predch·dzaj˙ci bod je vo vnutri obdÂûnika
				{
					QPoint P(xMin[j], S.y() + (xMin[j] - S.x()) * ((V[i].y() - S.y()) / (double)(V[i].x() - S.x())));
					W.push_back(P); // Pridaj nov˝ bod
				}
			}
			S = V[i];  // NastavÌme predch·dzaj˙ci bod na aktu·lny bod
		}
		V = W; // NastavÌme polygon na nov˝ polygon
		W.clear(); // VyËistÌme pomocn˝ vektor bodov

		// OtoËÌme vektor bodov o 90 stupÚov
		for (int k = 0; k < V.size(); k++)
		{
			QPoint swap = V[k];
			V[k].setX(swap.y());
			V[k].setY(-swap.x());
		}
	}
	return V; // Vr·time orezan˝ polygon
}


int crossProduct(QPoint v1, QPoint v2)
{
	return v1.x() * v2.y() - v1.y() * v2.x();
}
//orezavanie ciary
std::vector<QPoint> ViewerWidget::clipLineByPolygon(std::vector<QPoint> points, QColor color) {
	// ⁄seËka musÌ maù 2 body, inak vr·tiù pr·zdny vektor bodov
	if (points.size() < 2) {
		return std::vector<QPoint>();
	}

	std::vector<QPoint> clippedPoints; // Vektor pre orezanÈ body ˙seËky
	QPoint P1 = points[0], P2 = points[1]; // Body ˙seËky
	double t_min = 0, t_max = 1; // PoËiatoËnÈ hodnoty parametrov t_min a t_max
	QPoint d = P2 - P1; // Vektor smeru ˙seËky

	// DefinÌcia hranÌc obrazovky, do ktorej chceme orezaù ˙seËku
	std::vector<QPoint> E = { QPoint(5,5), QPoint(495,5), QPoint(495,495), QPoint(5,495) };

	// Prech·dzanie cez kaûd˙ hranicu obrazovky
	for (int i = 0; i < E.size(); i++) {
		QPoint E1 = E[i]; // ZaËiatoËn˝ bod hrany
		QPoint E2 = E[(i + 1) % E.size()]; // Koncov˝ bod hrany

		// V˝poËet norm·ly pre aktu·lnu hranu
		QPoint normal = QPoint(E2.y() - E1.y(), E1.x() - E2.x());

		// Ak je smer norm·ly opaËn˝, pretoûe potrebujeme norm·lu smeruj˙cu von z polygonu
		if (crossProduct(E2 - E1, normal) < 0) {
			normal = QPoint(-normal.y(), normal.x());
		}

		// VypoËÌtanie s˙Ëinu medzi smerom ˙seËky a norm·lou hrany
		double dn = d.x() * normal.x() + d.y() * normal.y();
		// VypoËÌtanie s˙Ëinu medzi vektorom medzi poËiatkom ˙seËky a poËiatkom hrany a norm·lou hrany
		double wn = (P1 - E1).x() * normal.x() + (P1 - E1).y() * normal.y();

		// Ak nie je s˙Ëin nulov˝, mÙûeme pokraËovaù v orezanÌ
		if (dn != 0)
		{
			double t = -wn / dn; // VypoËÌtanie parametra t
			if (dn > 0 && t <= 1) { // Ak je norm·la orientovan· smerom ˙seËky a t je v rozsahu <0,1>, uprav t_min
				t_min = std::max(t, t_min);
			}
			else if (dn < 0 && t >= 0) { // Ak je norm·la orientovan· opaËn˝m smerom, uprav t_max
				t_max = std::min(t, t_max);
			}
		}
	}
	/*
	if (t_min == 0 && t_max == 1) {
		// ⁄seËka je ˙plne mimo obrazovky
		return std::vector<QPoint>();
	}
	*/
	if (t_min < t_max) {
		// VypoËÌtanie orezan˝ch bodov a pridanie ich do v˝slednÈho vektoru
		QPoint clippedP1 = P1 + (P2 - P1) * t_min;
		QPoint clippedP2 = P1 + (P2 - P1) * t_max;

		clippedPoints.push_back(clippedP1);
		clippedPoints.push_back(clippedP2);
	}

	return clippedPoints; // Vr·tiù vektor orezan˝ch bodov ˙seËky
}



class hrana {

private:
	QPointF startPoint_;
	QPointF endPoint_;
	double smernica_;
	int deltaY_;
	double x_;
	double w_;


public:
	hrana(QPointF s, QPointF e, double smernica)
	{
		startPoint_ = s;
		endPoint_ = e;
		smernica_ = smernica;
	}

	hrana() {};
	QPointF startPoint() { return startPoint_; };
	QPointF endPoint() { return endPoint_; };
	void setStartPoint(QPointF point) { startPoint_ = point; };
	void setEndPoint(QPointF point) { endPoint_ = point; };
	double smernica() { return smernica_; };
	void setSmernica(double smer) { smernica_ = smer; };

	void swapStartEndPoints() {
		std::swap(startPoint_, endPoint_);
	}

	int deltaY() { return deltaY_; };
	double x() { return x_; };
	double w() { return w_; };
	void setDeltaY(int y) { deltaY_ = y; };
	void setW(double w) { w_ = w; };
	void setX(double x) { x_ = x; };

	// Metoda pro obr·cenÌ smÏrnice hrany
	void reverseSmernica() {
		smernica_ = -smernica_;
	}
};

bool porovnaniePodlaY(hrana hrana1, hrana hrana2) {
	return hrana1.startPoint().y() < hrana2.startPoint().y();
}
bool porovnaniePodlaX(hrana hrana1, hrana hrana2) {
	return hrana1.x() < hrana2.x();
}

//pomocna funkcia na nacitanie hran pred vyfarbenim polygonu
QVector<hrana> nacitajHrany(const std::vector<QPoint>& points) {
	QVector<hrana> hrany;

	for (int i = 0; i < points.size(); i++)
	{
		QPoint startPoint = points[i];
		QPoint endPoint = points[(i + 1) % points.size()];
		if (startPoint.y() != endPoint.y())
		{
			if (startPoint.y() > endPoint.y())
			{
				QPoint p = startPoint;
				startPoint = endPoint;
				endPoint = p;
			}
			double delenie = (endPoint.x() - startPoint.x());
			if (delenie == 0)
			{
				delenie = 1 / DBL_MAX;
			}
			double smernica = (double)(endPoint.y() - startPoint.y()) / delenie;
			endPoint.setY(endPoint.y() - 1);
			hrana hr(startPoint, endPoint, smernica);
			hrany.push_back(hr);

		}

	}
	std::sort(hrany.begin(), hrany.end(), porovnaniePodlaY);
	return hrany;
}

//vyfarbenie polygonu
void ViewerWidget::fillPolygon(std::vector<QPoint> points, QColor color, double zValue)
{
	if (points.size() < 3) {
		return;
	}
	QVector<hrana> hrany = nacitajHrany(points);

	if (hrany.size() < 2)
	{
		return;
	}

	int yMin = hrany.front().startPoint().y();
	int yMax = hrany.front().endPoint().y();

	for (int i = 1; i < hrany.size(); i++)
	{
		if (hrany[i].endPoint().y() > yMax)
		{
			yMax = hrany[i].endPoint().y();
		}
	}
	//experimentalne zistene, ze s +2 to funguje ;)
	std::vector<std::vector<hrana>> TH(yMax - yMin + 2);

	if (TH.size() < 2)
	{
		return;
	}

	for (int i = 0; i < hrany.size(); i++)
	{
		int deltaY = hrany[i].endPoint().y() - hrany[i].startPoint().y();
		double x = hrany[i].startPoint().x();
		double w = 0;
		if (hrany[i].smernica() == 0)
		{
			w = 0;
		}
		else
		{
			w = 1.0 / hrany[i].smernica();
		}

		hrany[i].setDeltaY(deltaY);
		hrany[i].setW(w);
		hrany[i].setX(x);

		TH[(hrany[i].startPoint().y() - yMin)].push_back(hrany[i]);
	}

	std::vector<hrana> ZAH;

	int y = yMin;
	for (int i = 0; i < yMax - yMin; i++)
	{
		if (!TH[i].empty()) {
			for (const hrana& edge : TH[i]) {
				ZAH.push_back(edge);
			}
		}
		std::sort(ZAH.begin(), ZAH.end(), porovnaniePodlaX);

		for (int j = 0; j < ZAH.size(); j += 2)
		{
			if (j + 1 < ZAH.size() && ZAH[j].x() != ZAH[j + 1].x())
			{
				for (int k = round(ZAH[j].x()); k <= round(ZAH[j + 1].x()); k++)
				{
					if (isInside(k, y))
					{
						setPixel(k, y, color);
					}

				}
			}
		}
		for (int j = 0; j < ZAH.size(); j++)
		{
			if (ZAH[j].deltaY() == 0)
			{
				ZAH.erase(ZAH.begin() + j);
				j--;
			}
		}
		for (int j = 0; j < ZAH.size(); j++)
		{
			ZAH[j].setDeltaY(ZAH[j].deltaY() - 1);
			ZAH[j].setX(ZAH[j].x() + ZAH[j].w());
		}
		y++;
	}
}

bool pointComparator(QPoint& p1, const QPoint& p2) {
	if (p1.y() != p2.y()) {
		return p1.y() < p2.y();
	}
	else {
		return p1.x() < p2.x();
	}
}

//vyplnenie trojuholnika
void ViewerWidget::fillTriangle(std::vector<QPoint> points, QColor color, int algType)
{
	if (points.size() > 3)
	{
		return;
	}
	std::vector<QPoint> T = points;
	std::sort(T.begin(), T.end(), pointComparator);

	if (T[0].y() == T[1].y())
	{
		fill(T[0], T[1], T[2], color);
	}
	else if (T[1].y() == T[2].y())
	{
		fill(T[0], T[1], T[2], color);
	}
	else
	{
		double m = (T[2].y() - T[0].y()) / (double)(T[2].x() - T[0].x());
		QPoint P((T[1].y() - T[0].y()) / m + T[0].x(), T[1].y());
		drawLine(T[1], P, color, algType);
		if (T[1].x() < P.x())
		{
			fill(T[0], T[1], P, color);
			fill(T[1], P, T[2], color);
		}
		else
		{
			fill(T[0], P, T[1], color);
			fill(P, T[1], T[2], color);
		}
	}
}

//funkcia na pripocitanie farby k farbe
QColor addColors(const QColor& color1, const QColor& color2)
{
	int red = qBound(0, color1.red() + color2.red(), 255);
	int green = qBound(0, color1.green() + color2.green(), 255);
	int blue = qBound(0, color1.blue() + color2.blue(), 255);
	int alpha = qBound(0, color1.alpha() + color2.alpha(), 255);

	return QColor(red, green, blue, alpha);
}

//funkcia na vynasobenie farby skalarom
QColor multiplyColorByScalar(const QColor& color, qreal scalar)
{
	int red = qBound(0, qRound(color.red() * scalar), 255);
	int green = qBound(0, qRound(color.green() * scalar), 255);
	int blue = qBound(0, qRound(color.blue() * scalar), 255);
	int alpha = qBound(0, qRound(color.alpha() * scalar), 255);

	return QColor(red, green, blue, alpha);
}

//vyplnanie trojuholnika
void ViewerWidget::fill(QPoint point1, QPoint point2, QPoint point3, QColor color)
{
	QPoint points[3] = { point1, point2, point3 };
	std::vector<QPoint> triangle;

	if (newTriangle == true)
	{
		triangle = newTrianglePoints;
	}
	else
	{
		triangle = polygon_points_;
	}
	std::vector<hrana> hrany;
	std::vector <QColor> colors = { fillColor, fillColor2, fillColor3 };
	if (typeOffilling_ == 0 || typeOffilling_ == 1)
	{
		if (isInside(triangle[0].x(), triangle[0].y()))
		{
			setPixel(triangle[0].x(), triangle[0].y(), colors[0]);
		}
		if (isInside(triangle[1].x(), triangle[1].y()))
		{
			setPixel(triangle[1].x(), triangle[1].y(), colors[1]);
		}
		if (isInside(triangle[2].x(), triangle[2].y()))
		{
			setPixel(triangle[2].x(), triangle[2].y(), colors[2]);
		}
	}
	for (int i = 0; i < 3; i++)
	{
		int nextIndex = (i + 1) % 3;

		double delenie = (points[nextIndex].x() - points[i].x());
		if (delenie == 0)
		{
			delenie = 1 / DBL_MAX;
		}
		if ((points[i].y() - points[nextIndex].y()) != 0)
		{
			double smernica = (double)(points[nextIndex].y() - points[i].y()) / delenie;
			hrany.push_back(hrana(points[i], points[nextIndex], smernica));
		}
	}
	if (hrany.size() == 0)
	{
		return;
	}
	hrana e1 = hrany[0];
	hrana e2 = hrany[1];

	double obsahA;
	if (typeOffilling_ == 1)
	{
		QPoint u = triangle[1] - triangle[0];
		QPoint v = triangle[2] - triangle[0];
		obsahA = fabs(u.x() * v.y() - u.y() * v.x()) / 2.0;
	}

	if (e1.startPoint().y() > e1.endPoint().y())
	{
		e1.swapStartEndPoints();

		double delenie = (e1.endPoint().x() - e1.startPoint().x());
		if (delenie == 0)
		{
			delenie = 1 / DBL_MAX;
		}
		double smernica = (double)(e1.endPoint().y() - e1.startPoint().y()) / delenie;

		e1.setSmernica(smernica);
	}
	if (e2.startPoint().y() > e2.endPoint().y())
	{
		e2.swapStartEndPoints();
		double delenie = (e2.endPoint().x() - e2.startPoint().x());
		if (delenie == 0)
		{
			delenie = 1 / DBL_MAX;
		}
		double smernica = (double)(e2.endPoint().y() - e2.startPoint().y()) / delenie;
		e2.setSmernica(smernica);
	}
	if (e1.startPoint().x() > e2.startPoint().x())
	{
		std::swap(e1, e2);
	}

	double w1 = 1 / e1.smernica();
	double w2 = 1 / e2.smernica();

	double y = e1.startPoint().y();
	double yMax = e1.endPoint().y();
	double x1 = e1.startPoint().x();
	double x2 = e2.startPoint().x();

	while (y < yMax)
	{
		if (x1 != x2)
		{
			for (double i = x1; i < x2; i++)
			{
				if (isInside(i, y))
				{
					if (typeOffilling_ == 0)
					{
						double D0 = sqrt(pow(triangle[0].x() - i, 2) + pow(triangle[0].y() - y, 2));
						double D1 = sqrt(pow(triangle[1].x() - i, 2) + pow(triangle[1].y() - y, 2));
						double D2 = sqrt(pow(triangle[2].x() - i, 2) + pow(triangle[2].y() - y, 2));
						if (D0 < D1 && D0 < D2)
						{
							color = colors[0];
						}
						if (D1 < D0 && D1 < D2)
						{
							color = colors[1];
						}
						if (D2 < D0 && D2 < D1)
						{
							color = colors[2];
						}
					}
					else if (typeOffilling_ == 1)
					{
						//((point1.x() - i)* (point2.y() - y) - (point1.y() - y) * (point2.x() - i)) / 2.0;
						double obsahA0 = fabs((triangle[1].x() - i) * (triangle[2].y() - y) - (triangle[1].y() - y) * (triangle[2].x() - i)) / 2.0;
						double obsahA1 = fabs((triangle[0].x() - i) * (triangle[2].y() - y) - (triangle[0].y() - y) * (triangle[2].x() - i)) / 2.0;
						double lambda0 = obsahA0 / obsahA;
						double lambda1 = obsahA1 / obsahA;
						double lambda2 = 1 - lambda0 - lambda1;
						color = addColors(multiplyColorByScalar(colors[0], lambda0),
							addColors(multiplyColorByScalar(colors[1], lambda1),
								multiplyColorByScalar(colors[2], lambda2)));

					}
					else if (typeOffilling_ == 2)
					{
						color = fillColor;
					}

					setPixel(i, y, color);
				}
			}
		}
		x1 += w1;
		x2 += w2;
		y++;
	}

}

QPoint tangent(const QPoint& P, double slope, double length) {
	double angle = atan(slope);
	double dx = length * cos(angle);
	double dy = length * sin(angle);
	return QPoint(/*P.x() + */ dx, /*/P.y() + */dy);
}


//hermit krivka
void ViewerWidget::hermit(QColor color, int algType) {

	double deltaT = 0.0001;
	QPoint Q0, Q1;
	double t;

	for (int i = 1; i < curvePoints_.size(); i++) {
		Q0 = curvePoints_[i - 1];
		t = deltaT;

		while (t < 1) {
			double F0 = 2 * pow(t, 3) - 3 * pow(t, 2) + 1;
			double F1 = -2 * pow(t, 3) + 3 * pow(t, 2);
			double F2 = pow(t, 3) - 2 * pow(t, 2) + t;
			double F3 = pow(t, 3) - pow(t, 2);

			QPoint P0 = tangent(curvePoints_[i - 1], smernice_[i - 1], 70);
			QPoint P1 = tangent(curvePoints_[i], smernice_[i], 70);

			Q1 = curvePoints_[i - 1] * F0 + curvePoints_[i] * F1 + P0 * F2 + P1 * F3;

			drawLine(Q0, Q1, color, 0);
			Q0 = Q1;
			t += deltaT;
		}
		drawLine(Q0, curvePoints_[i], color, algType);
	}
}

//bezier krivka
void ViewerWidget::bezier(QColor color, int algType)
{
	std::vector<std::vector<QPoint>> pole;
	pole.resize(curvePoints_.size(), std::vector<QPoint>(curvePoints_.size()));

	for (int i = 0; i < curvePoints_.size(); i++)
	{
		pole[0][i] = curvePoints_[i];
	}

	double deltaT = 0.0001;
	double t = deltaT;
	QPoint Q0 = curvePoints_[0], Q1;

	while (t < 1)
	{
		for (int i = 1; i < curvePoints_.size(); i++)
		{
			for (int j = 0; j < curvePoints_.size() - i; j++)
			{
				pole[i][j] = (1 - t) * pole[i - 1][j] + t * pole[i - 1][j + 1];
			}
		}
		Q1 = pole[curvePoints_.size() - 1][0];
		drawLine(Q0, Q1, color, algType);
		Q0 = Q1;
		t += deltaT;
	}
	drawLine(Q0, curvePoints_[curvePoints_.size() - 1], color, algType);
}

//coonsov krivka
void ViewerWidget::coonsov(QColor color, int algType)
{
	double deltaT = 0.0001;

	for (int i = 3; i < curvePoints_.size(); i++)
	{
		double t = 0;
		QPoint Q0;

		while (t < 1)
		{
			double B0 = ((-1 / 6.0) * pow(t, 3)) + ((1 / 2.0) * pow(t, 2)) - ((1 / 2.0) * t) + 1 / 6.0;
			double B1 = ((1 / 2.0) * pow(t, 3)) - pow(t, 2) + 2 / 3.0;
			double B2 = -1 / 2.0 * pow(t, 3) + 1 / 2.0 * pow(t, 2) + 1 / 2.0 * t + 1 / 6.0;
			double B3 = 1 / 6.0 * pow(t, 3);

			Q0 = curvePoints_[i - 3] * B0 + curvePoints_[i - 2] * B1 + curvePoints_[i - 1] * B2 + curvePoints_[i] * B3;

			t += deltaT;

			double B0_next = ((-1 / 6.0) * pow(t, 3)) + ((1 / 2.0) * pow(t, 2)) - ((1 / 2.0) * t) + 1 / 6.0;
			double B1_next = ((1 / 2.0) * pow(t, 3)) - pow(t, 2) + 2 / 3.0;
			double B2_next = -1 / 2.0 * pow(t, 3) + 1 / 2.0 * pow(t, 2) + 1 / 2.0 * t + 1 / 6.0;
			double B3_next = 1 / 6.0 * pow(t, 3);

			QPoint Q1 = curvePoints_[i - 3] * B0_next + curvePoints_[i - 2] * B1_next + curvePoints_[i - 1] * B2_next + curvePoints_[i] * B3_next;

			drawLine(Q0, Q1, color, algType);
		}
	}
}


void ViewerWidget::pohladovaSustava(double zenit, double azimut, bool chcemStredove, double vzdialenost)
{
	zhenit = zenit;
	azhimut = azimut;
	double teta = (zenit * M_PI) / 180.0;
	double fi = (azimut * M_PI) / 180.0;

	double nx = sin(teta) * sin(fi);
	double ny = sin(teta) * cos(fi);
	double nz = cos(teta);
	bazoveVektory[2] = QVector3D(nx, ny, nz).normalized();

	double ux = sin(teta + M_PI_2) * sin(fi);
	double uy = sin(teta + M_PI_2) * cos(fi);
	double uz = cos(teta + M_PI_2);
	bazoveVektory[1] = QVector3D(ux, uy, uz);

	QVector3D v = QVector3D::crossProduct(bazoveVektory[1], bazoveVektory[2]);
	bazoveVektory[0] = v;

	nove3DSuradnice(chcemStredove, vzdialenost);
}


void ViewerWidget::nove3DSuradnice(bool chcemStredove, double vzdialenost)
{
	for (int i = 0; i < hranyKocky.size(); i++)
	{
		QVector3D point1(hranyKocky[i].vrchol_prvy->x, hranyKocky[i].vrchol_prvy->y, hranyKocky[i].vrchol_prvy->z);
		QVector3D point2(hranyKocky[i].vrchol_druhy->x, hranyKocky[i].vrchol_druhy->y, hranyKocky[i].vrchol_druhy->z);

		novaKocka[i].vrchol_prvy->x = QVector3D::dotProduct(point1, bazoveVektory[0]);
		novaKocka[i].vrchol_prvy->y = QVector3D::dotProduct(point1, bazoveVektory[1]);
		novaKocka[i].vrchol_prvy->z = QVector3D::dotProduct(point1, bazoveVektory[2]);

		novaKocka[i].vrchol_druhy->x = QVector3D::dotProduct(point2, bazoveVektory[0]);
		novaKocka[i].vrchol_druhy->y = QVector3D::dotProduct(point2, bazoveVektory[1]);
		novaKocka[i].vrchol_druhy->z = QVector3D::dotProduct(point2, bazoveVektory[2]);

		if (chcemStredove)
		{
			novaKocka[i].vrchol_prvy->x = (novaKocka[i].vrchol_prvy->x * vzdialenost) / fabs(vzdialenost - novaKocka[i].vrchol_prvy->z);
			novaKocka[i].vrchol_prvy->y = (novaKocka[i].vrchol_prvy->y * vzdialenost) / fabs(vzdialenost - novaKocka[i].vrchol_prvy->z);
			//novaKocka[i].vrchol_prvy->z = fabs(vzdialenost - novaKocka[i].vrchol_prvy->z);

			novaKocka[i].vrchol_druhy->x = (novaKocka[i].vrchol_druhy->x * vzdialenost) / fabs(vzdialenost - novaKocka[i].vrchol_druhy->z);
			novaKocka[i].vrchol_druhy->y = (novaKocka[i].vrchol_druhy->y * vzdialenost) / fabs(vzdialenost - novaKocka[i].vrchol_druhy->z);
			//novaKocka[i].vrchol_druhy->z = fabs(vzdialenost - novaKocka[i].vrchol_druhy->z);
		}
	}

}


struct UniqueSurface {
	std::vector<QVector3D> vertices;
	QColor color;
};

std::vector<UniqueSurface> findUniqueSurfaces(std::vector<Hrana>& hranyKocky) {
	std::unordered_map<Plocha*, std::pair<std::vector<Vrchol*>, QColor>> surfaceDataMap;

	for (const auto& hrana : hranyKocky) {
		for (const auto& plocha : hrana.plocha) {
			if (surfaceDataMap.find(plocha) == surfaceDataMap.end()) {
				surfaceDataMap[plocha] = { {hrana.vrchol_prvy, hrana.vrchol_druhy}, plocha->farbaOlochy };
			}
			else {
				auto& surfaceData = surfaceDataMap[plocha];
				auto& vertices = surfaceData.first;
				if (vertices.size() < 3) {
					if (std::find(vertices.begin(), vertices.end(), hrana.vrchol_prvy) == vertices.end()) {
						vertices.push_back(hrana.vrchol_prvy);
					}
					if (std::find(vertices.begin(), vertices.end(), hrana.vrchol_druhy) == vertices.end()) {
						vertices.push_back(hrana.vrchol_druhy);
					}
				}
			}
		}
	}

	std::vector<UniqueSurface> uniqueSurfaces;
	for (const auto& pair : surfaceDataMap) {
		const auto& vertices = pair.second.first;
		// Ensure each surface has exactly three vertices
		if (vertices.size() == 3) {
			UniqueSurface surface;
			for (const auto& vertex : vertices) {
				surface.vertices.push_back(QVector3D(vertex->x + 250, vertex->y + 250, vertex->z));
			}
			surface.color = pair.second.second;
			uniqueSurfaces.push_back(surface);
		}
	}

	return uniqueSurfaces;
}

//vykreslenie kocky s danymi novymi suradnicami
void ViewerWidget::premietanie(bool chcemVyfarbit)
{
	initHlbkaPixelov();

	if (chcemVyfarbit)
	{
		std::vector<UniqueSurface> uniqueSurfaces = findUniqueSurfaces(novaKocka);
		std::vector<std::vector<QVector3D>> bodyPlochy;
		std::vector<QColor> farbyPlochy;
		for (int i = 0; i < uniqueSurfaces.size(); i++)
		{
			bodyPlochy.push_back(uniqueSurfaces[i].vertices);
			farbyPlochy.push_back(uniqueSurfaces[i].color);
		}
		for (int i = 0;i < bodyPlochy.size();i++)
		{
			fillTriangle3D(bodyPlochy[i], farbyPlochy[i], 0, false);
			//fill_triangle3D_depth(bodyPlochy[i], farbyPlochy[i], false);
		}

	}

	for (int i = 0; i < novaKocka.size(); i++)
	{
		QVector3D point1(novaKocka[i].vrchol_prvy->x + 250, novaKocka[i].vrchol_prvy->y + 250, novaKocka[i].vrchol_prvy->z);
		QVector3D point2(novaKocka[i].vrchol_druhy->x + 250, novaKocka[i].vrchol_druhy->y + 250, novaKocka[i].vrchol_druhy->z);

		//drawLine(startPoint, endPoint, Qt::red, 1);
		drawLine3D(point1, point2, Qt::black, 0);
	}



}

//precitanie VTK suboru a ulozenie do objektu
void ViewerWidget::ulozKockuZoSuboru(std::wstring filename)
{
	srand(time(NULL));
	std::ifstream file(filename);

	if (!file.is_open()) {
		std::wcerr << L"Chyba pri otv·ranÌ s˙boru " << filename << std::endl;
		return;
	}

	std::string line;

	bool reachedPoints = false;
	bool reachedLines = false;
	bool reachedPolygons = false;

	while (std::getline(file, line))
	{
		if (!reachedPoints) {
			if (line.find("POINTS") != std::string::npos) {
				reachedPoints = true;
				continue;
			}
		}
		else if (!reachedLines) {
			if (line.find("LINES") != std::string::npos) {
				reachedLines = true;
				continue;
			}
		}
		else if (!reachedPolygons) {
			if (line.find("POLYGONS") != std::string::npos) {
				reachedPolygons = true;
				continue;
			}
		}
		if (reachedPoints && !reachedLines && !reachedPolygons)
		{
			std::istringstream iss(line);
			Vrchol vrchol;
			iss >> vrchol.x >> vrchol.y >> vrchol.z;
			vrcholy.push_back(vrchol);
			vrcholy1.push_back(vrchol);
		}
		if (reachedLines && !reachedPolygons)
		{
			std::istringstream iss(line);
			int nic, prvyVrchol, druhyVrchol;
			iss >> nic >> prvyVrchol >> druhyVrchol;
			Hrana e;
			e.vrchol_prvy = &vrcholy[prvyVrchol];
			e.vrchol_druhy = &vrcholy[druhyVrchol];
			hranyKocky.push_back(e);

			Hrana e1;
			e1.vrchol_prvy = &vrcholy1[prvyVrchol];
			e1.vrchol_druhy = &vrcholy1[druhyVrchol];

			novaKocka.push_back(e1);
		}
		if (reachedPolygons)
		{
			std::istringstream iss(line);
			int pocetBodov, bod1, bod2, bod3, bod4;
			iss >> pocetBodov >> bod1 >> bod2 >> bod3 /* >> bod4*/;

			Plocha* p = new Plocha();
			Plocha* p1 = new Plocha();
			std::vector<Vrchol*> vrchol;


			for (int i = 0; i < vrcholy.size(); i++)
			{
				if (bod1 == i || bod2 == i || bod3 == i /* || bod4 == i*/)
				{
					vrchol.push_back(&vrcholy[i]);
				}
			}
			for (int i = 0; i < hranyKocky.size(); i++) {
				if ((vrchol[0] == hranyKocky[i].vrchol_prvy && vrchol[1] == hranyKocky[i].vrchol_druhy) ||
					(vrchol[1] == hranyKocky[i].vrchol_prvy && vrchol[0] == hranyKocky[i].vrchol_druhy) ||
					(vrchol[1] == hranyKocky[i].vrchol_prvy && vrchol[2] == hranyKocky[i].vrchol_druhy) ||
					(vrchol[2] == hranyKocky[i].vrchol_prvy && vrchol[1] == hranyKocky[i].vrchol_druhy) ||
					(vrchol[2] == hranyKocky[i].vrchol_prvy && vrchol[0] == hranyKocky[i].vrchol_druhy) ||
					(vrchol[0] == hranyKocky[i].vrchol_prvy && vrchol[2] == hranyKocky[i].vrchol_druhy)
					)
				{
					p->hrana.push_back(&hranyKocky[i]);
					p1->hrana.push_back(&novaKocka[i]);
				}

				/*
				//ak su plochy stvoruholniky
				else if (vrchol[2] == hranyKocky[i].vrchol_prvy && vrchol[3] == hranyKocky[i].vrchol_druhy) {
					p->hrana.push_back(&hranyKocky[i]);
					p1->hrana.push_back(&novaKocka[i]);
				}
				else if (vrchol[3] == hranyKocky[i].vrchol_prvy && vrchol[2] == hranyKocky[i].vrchol_druhy) {
					p->hrana.push_back(&hranyKocky[i]);
					p1->hrana.push_back(&novaKocka[i]);
				}

				else if (vrchol[3] == hranyKocky[i].vrchol_prvy && vrchol[0] == hranyKocky[i].vrchol_druhy) {
					p->hrana.push_back(&hranyKocky[i]);
					p1->hrana.push_back(&novaKocka[i]);
				}
				else if (vrchol[0] == hranyKocky[i].vrchol_prvy && vrchol[3] == hranyKocky[i].vrchol_druhy) {
					p->hrana.push_back(&hranyKocky[i]);
					p1->hrana.push_back(&novaKocka[i]);
				}
				*/
			}
			p->farbaOlochy = QColor(rand() % 256, rand() % 256, rand() % 256);
			p1->farbaOlochy = QColor(rand() % 256, rand() % 256, rand() % 256);

			for (int i = 0; i < p->hrana.size(); i++) {
				for (int j = 0; j < hranyKocky.size(); j++) {
					if (p->hrana[i] == &hranyKocky[j]) {
						hranyKocky[j].plocha.push_back(p);
						novaKocka[j].plocha.push_back(p1);
						break;
					}
				}
			}
		}
	}

	file.close();
}



bool pointComparator2(const QVector3D& p1, const QVector3D& p2) {
	if (p1.y() != p2.y()) {
		return p1.y() < p2.y();
	}
	else {
		return p1.x() < p2.x();
	}
}

//vyplnenie trojuholnika
void ViewerWidget::fillTriangle3D(std::vector<QVector3D>& points, QColor color, int algType, bool phong, int typeOfFilling)
{
	if (points.size() > 3)
	{
		return;
	}
	std::vector<QVector3D>& T = points;
	trianglePoints = { points[0], points[1], points[2] };
	std::sort(T.begin(), T.end(), pointComparator2);


	// Proceed with your existing logic using sortedPoints and sortedZValues
	if (T[0].y() == T[1].y()) {
		fill3D(T[0], T[1], T[2], color, phong, typeOfFilling);
	}
	else if (T[1].y() == T[2].y()) {
		fill3D(T[0], T[1], T[2], color, phong, typeOfFilling);
	}
	else
	{
		double m = (T[2].y() - T[0].y()) / (double)(T[2].x() - T[0].x());


		double xP = (double)((T[1].y() - T[0].y()) / m) + T[0].x();
		double yP = T[1].y();
		double zP =
			T[0].z() +
			((xP - T[0].x()) * (T[1].z() - T[0].z()) /
				(T[1].x() - T[0].x())) +
			((yP - T[0].y()) * (T[1].z() - T[0].z()) /
				(T[1].y() - T[0].y()));
		QVector3D P(xP, yP, zP);
		std::vector<QVector3D> top_triangle, bottom_triangle;
		if (T[1].x() < P.x()) {
			top_triangle = { T[0], T[1], P };
			bottom_triangle = { T[1], P, T[2] };
		}
		else {
			top_triangle = { T[0], P, T[1] };
			bottom_triangle = { P, T[1], T[2] };
		}

		// Fill top and bottom triangles
		fill3D(top_triangle[0], top_triangle[1], top_triangle[2], color, phong, typeOfFilling);
		fill3D(bottom_triangle[0], bottom_triangle[1], bottom_triangle[2], color, phong, typeOfFilling);
	}
}



void ViewerWidget::fill3D(QVector3D point1, QVector3D point2, QVector3D point3, QColor color, bool phong, int typeOfFilling)
{
	std::vector<QVector3D> triangle = { point1,point2,point3 };
	std::vector <QColor> colors = { fillColor, fillColor2, fillColor3 };

	if (phong && typeOfFilling == 1)
	{
		colors[0] = vratFarbu(trianglePoints[0]);
		colors[1] = vratFarbu(trianglePoints[1]);
		colors[2] = vratFarbu(trianglePoints[2]);
	}
	
	class hrana3D {

	private:
		QVector3D startPoint_;
		QVector3D endPoint_;
		double smernica_;
		int deltaY_;
		double x_;
		double w_;


	public:
		hrana3D(QVector3D s, QVector3D e, double smernica)
		{
			startPoint_ = s;
			endPoint_ = e;
			smernica_ = smernica;
		}

		hrana3D() {};
		QVector3D startPoint() { return startPoint_; };
		QVector3D endPoint() { return endPoint_; };
		void setStartPoint(QVector3D point) { startPoint_ = point; };
		void setEndPoint(QVector3D point) { endPoint_ = point; };
		double smernica() { return smernica_; };
		void setSmernica(double smer) { smernica_ = smer; };

		void swapStartEndPoints() {
			std::swap(startPoint_, endPoint_);
		}

		int deltaY() { return deltaY_; };
		double x() { return x_; };
		double w() { return w_; };
		void setDeltaY(int y) { deltaY_ = y; };
		void setW(double w) { w_ = w; };
		void setX(double x) { x_ = x; };

	};

	std::vector<hrana3D> hrany;

	for (int i = 0; i < 3; i++)
	{
		int nextIndex = (i + 1) % 3;

		double delenie = (triangle[nextIndex].x() - triangle[i].x());
		if (delenie == 0)
		{
			delenie = 1 / DBL_MAX;
		}
		if ((triangle[i].y() - triangle[nextIndex].y()) != 0)
		{
			double smernica = (double)(triangle[nextIndex].y() - triangle[i].y()) / delenie;
			hrany.push_back(hrana3D(triangle[i], triangle[nextIndex], smernica));
		}
	}
	if (hrany.size() == 0)
	{
		return;
	}
	hrana3D e1 = hrany[0];
	hrana3D e2 = hrany[1];

	double obsahA;

	QVector3D u = trianglePoints[1] - trianglePoints[0];
	QVector3D v = trianglePoints[2] - trianglePoints[0];
	obsahA = fabs(u.x() * v.y() - u.y() * v.x()) / 2.0;


	if (e1.startPoint().y() > e1.endPoint().y())
	{
		e1.swapStartEndPoints();

		double delenie = (e1.endPoint().x() - e1.startPoint().x());
		if (delenie == 0)
		{
			delenie = 1 / DBL_MAX;
		}
		double smernica = (double)(e1.endPoint().y() - e1.startPoint().y()) / delenie;

		e1.setSmernica(smernica);
	}
	if (e2.startPoint().y() > e2.endPoint().y())
	{
		e2.swapStartEndPoints();

		double delenie = (e2.endPoint().x() - e2.startPoint().x());
		if (delenie == 0)
		{
			delenie = 1 / DBL_MAX;
		}
		double smernica = (double)(e2.endPoint().y() - e2.startPoint().y()) / delenie;
		e2.setSmernica(smernica);
	}
	if (e1.startPoint().x() > e2.startPoint().x())
	{
		std::swap(e1, e2);
	}

	double w1 = 1 / e1.smernica();
	double w2 = 1 / e2.smernica();

	double y = e1.startPoint().y();
	double yMax = e1.endPoint().y();
	double x1 = e1.startPoint().x();
	double x2 = e2.startPoint().x();

	while (y < yMax)
	{
		if (x1 != x2)
		{
			for (double i = x1; i < x2; i++)
			{
				if (isInside(i, y))
				{
					double obsahA0 = fabs((trianglePoints[1].x() - i) * (trianglePoints[2].y() - y) - (trianglePoints[1].y() - y) * (trianglePoints[2].x() - i)) / 2.0;
					double obsahA1 = fabs((trianglePoints[0].x() - i) * (trianglePoints[2].y() - y) - (trianglePoints[0].y() - y) * (trianglePoints[2].x() - i)) / 2.0;

					double lambda0 = obsahA0 / obsahA * 1.0;
					double lambda1 = obsahA1 / obsahA * 1.0;
					double lambda2 = 1 - lambda0 - lambda1;

					double zValue = trianglePoints[0].z() * lambda0 + trianglePoints[1].z() * lambda1 + trianglePoints[2].z() * lambda2;

					if (phong)
					{
						if (typeOfFilling == 2 || typeOfFilling == 1)
							color = addColors(multiplyColorByScalar(colors[0], lambda0),
								addColors(multiplyColorByScalar(colors[1], lambda1),
									multiplyColorByScalar(colors[2], lambda2)));
						else if (typeOfFilling == 0)
							color = colors[0];
					}

					if (phong && typeOfFilling == 1)
					{
						double D0 = sqrt(pow(trianglePoints[0].x() - i, 2) + pow(trianglePoints[0].y() - y, 2));
						double D1 = sqrt(pow(trianglePoints[1].x() - i, 2) + pow(trianglePoints[1].y() - y, 2));
						double D2 = sqrt(pow(trianglePoints[2].x() - i, 2) + pow(trianglePoints[2].y() - y, 2));
						if (D0 < D1 && D0 < D2)
						{
							color = colors[0];
						}
						if (D1 < D0 && D1 < D2)
						{
							color = colors[1];
						}
						if (D2 < D0 && D2 < D1)
						{
							color = colors[2];
						}
					}

					setPixel3D((int)i, (int)y, color, zValue);
				}

			}
		}
		x1 += w1;
		x2 += w2;
		y++;
	}
}



//kreslenie ciary
void ViewerWidget::drawLine3D(QVector3D start, QVector3D end, QColor color, int algType)
{
	double dx = end.x() - start.x();
	double dy = end.y() - start.y();
	double dz = end.z() - start.z();


	//najprv treba polygon orezat - ak netreba, tak bude vrateny povodny
	std::vector<QVector3D> line = { start,end };

	QVector3D newStart = line[0];
	QVector3D newEnd = line[1];

	//DDA algoritmus
	if (algType == 0)
	{
		// kontrola, Ëi dx je vacsie ako dy - ciara je viac vodorovna ako zvisla (sklon je < 45)
		if (qAbs(dx) > qAbs(dy)) // smer x
		{
			// vymen koncovy a zaciatocny bod, ak ma zaciatocny bod vacsie suradnice
			if (newStart.x() > newEnd.x()) {
				std::swap(newStart, newEnd);
			}
			// Ak je dx 0 - nastavime ho na DBL_MAX, aby sme predisli deleniu 0
			if (dx == 0)
			{
				dx = DBL_MAX;
			}
			//vypocitame smernicu ciary
			double m = dy / dx;
			double m_z = dz / dx;
			// Inicializujeme aktualnu y-ovu suradnicu na y-ovu suradnicu prveho bodu
			double actualY = newStart.y();
			double actualZ = newStart.z();
			// Iterujeme cez x-ove suradnice od start.x() po end.x()
			for (int i = newStart.x(); i <= newEnd.x(); i++)
			{
				// vykreslime dany body bod - zaokruhlime y-ovu suradnicu
				setPixel3D(i, qRound(actualY), color, actualZ + 2);
				actualY += m;
				actualZ += m_z;
			}
		}
		else // smer y (sklon je > 45)
		{
			// vymen koncovy a zaciatocny bod, ak ma zaciatocny bod vacsie suradnice
			if (newStart.y() > newEnd.y()) {
				std::swap(newStart, newEnd);
			}
			// Ak je dy 0 - nastavime ho na DBL_MAX, aby sme predisli deleniu 0
			if (dy == 0)
			{
				dy = DBL_MAX;
			}
			//vypocitame smernicu ciary
			double m = dx / dy;
			double m_z = dz / dy;
			// Inicializujeme aktualnu x-ovu suradnicu na x-ovu suradnicu prveho bodu
			double actualX = newStart.x();
			double actualZ = newStart.z();
			// Iterujeme cez y-ove suradnice od start.y() po end.y()
			for (int i = newStart.y(); i <= newEnd.y(); i++)
			{
				setPixel3D(qRound(actualX), i, color, actualZ + 2);
				actualX += m;
				actualZ += m_z;
			}
		}
	}
	update();
}



QColor ViewerWidget::vratFarbu(QVector3D bod)
{
	bod = { bod.x() - 250, bod.y() - 250, bod.z()};
	QVector3D n = bod; //normalovy luc - kedze gula so stredom v 0 0 0 
	QVector3D l = (zdrojSvetla - bod); //svetelny luc
	n.normalize();
	l.normalize();
	QVector3D v = { -bod.x(), -bod.y(),(float)poziciaKamery - bod.z()}; // view - pozicia kamery
	QVector3D r = (2 * QVector3D::dotProduct(l, n) * n - l); //odrazeny luc

	r.normalize();
	n.normalize();
	l.normalize();
	v.normalize();

	QVector3D i_l = farbaLuca; //vektor farby dopadajuceho luca
	QVector3D r_s = odraz / 255.0; //koeficient odrazu
	QVector3D r_d = difuzia / 255.0; //difuzny reflection coefficient
	QVector3D r_a = ambient / 255.0;
	QVector3D i_o = farbySceny; //ambient light color
	double h = ostrost;

	// Ambient component
	QVector3D i_a = i_o * r_a;
	
	//difuzia
	double diffuseIntensity = std::max(QVector3D::dotProduct(l, n), 0.0f);
	QVector3D i_d = i_l * r_d * diffuseIntensity;
	
	//odraz
	double specularIntensity = pow(std::max(QVector3D::dotProduct(v, r), 0.0f), h);
	QVector3D i_s = i_l * r_s * specularIntensity;
	
	// Combine components
	QVector3D finalColor = i_a + i_d + i_s;
	int r_ = std::clamp((int)finalColor.x(), 0, 255);
	int g_ = std::clamp((int)finalColor.y(), 0, 255);
	int b_ = std::clamp((int)finalColor.z(), 0, 255);

	return QColor(r_, g_, b_);

}

void ViewerWidget::phongModel(bool phong, int typeOfFilling, bool chcemVyfarbit, bool tocitSvetlom)
{
	initHlbkaPixelov();
	
	std::vector<UniqueSurface> uniqueSurfaces = findUniqueSurfaces(novaKocka);
	std::vector<std::vector<QVector3D>> bodyPlochy;
	std::vector<QColor> farbyPlochy;
	for (int i = 0; i < uniqueSurfaces.size(); i++)
	{
		bodyPlochy.push_back(uniqueSurfaces[i].vertices);
		farbyPlochy.push_back(uniqueSurfaces[i].color);
	}

	if (chcemVyfarbit)
	{
		if (tocitSvetlom)
		{
			zdrojSvetla.setX(QVector3D::dotProduct(originalLightSource, bazoveVektory[0]));
			zdrojSvetla.setY(QVector3D::dotProduct(originalLightSource, bazoveVektory[1]));
			zdrojSvetla.setZ(QVector3D::dotProduct(originalLightSource, bazoveVektory[2]));
		}	

		for (int i = 0;i < bodyPlochy.size();i++)
		{
			std::vector<QColor> colors;
			QColor color = vratFarbu(bodyPlochy[i][0]);
			colors.push_back(color);
			color = vratFarbu(bodyPlochy[i][1]);
			colors.push_back(color);
			color = vratFarbu(bodyPlochy[i][2]);
			colors.push_back(color);


			fillColor = colors[0];
			fillColor2 = colors[1];
			fillColor3 = colors[2];
			fillTriangle3D(bodyPlochy[i], farbyPlochy[i], 0, phong, typeOfFilling);
		}
		zdrojSvetla = originalLightSource;
	}
	for (int i = 0; i < novaKocka.size(); i++)
	{
		QVector3D point1(novaKocka[i].vrchol_prvy->x + 250, novaKocka[i].vrchol_prvy->y + 250, novaKocka[i].vrchol_prvy->z);
		QVector3D point2(novaKocka[i].vrchol_druhy->x + 250, novaKocka[i].vrchol_druhy->y + 250, novaKocka[i].vrchol_druhy->z);

		//drawLine(startPoint, endPoint, Qt::red, 1);
		drawLine3D(point1, point2, Qt::black, 0);
	}

}
