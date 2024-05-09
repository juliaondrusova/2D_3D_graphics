#pragma once
#include <QtWidgets>
#include <iostream>
#include <vector>
#include <QVector3D>
#include <unordered_set>


class Hrana; // Deklaracia vopred
class Vrchol {

public:
	double x, y, z;
	QColor farbaVrchola;
};
class Plocha {

public:
	std::vector<Hrana*> hrana;
	bool jeVyfarbena = false;
	QColor farbaOlochy;

};
class Hrana {

public:
	Vrchol* vrchol_prvy, * vrchol_druhy;
	std::vector<Plocha*> plocha;

};


class ViewerWidget :public QWidget {
	Q_OBJECT

private:
	QSize areaSize = QSize(0, 0);
	QImage* img = nullptr;
	QPainter* painter = nullptr;
	uchar* data = nullptr;

	bool drawLineActivated = false;
	QPoint drawLineBegin = QPoint(0, 0);
	QPoint moveBegin = QPoint(0, 0);
	std::vector<QPoint> polygon_points_;
	std::vector<QPoint> line_points_;
	std::vector<QPoint> ellipseRadius_;
	std::vector<QPoint> curvePoints_;
	std::vector<double> smernice_;
	int typeOffilling_ = 0;
	std::vector<QPoint> newTrianglePoints;
	bool newTriangle = false;
	QColor fillColor;
	QColor fillColor2;
	QColor fillColor3;

public:
	//////////////////////3D GFAFIKA//////////////////////
	std::vector<Hrana> hranyKocky;
	std::vector<Hrana> novaKocka;
	std::vector<Vrchol> vrcholy;
	std::vector<Vrchol> vrcholy1;
	QVector3D bazoveVektory[3];
	void ulozKockuZoSuboru(std::wstring filename);
	void setPixel3D(int x, int y, const QColor& color, double zValue);
	std::vector<std::vector<double>> hlbkaPixelov;
	void fill3D(QVector3D point1, QVector3D point2, QVector3D point3, QColor color, bool phong, int typeOfFilling = 0);
	void fillTriangle3D(std::vector<QVector3D>& points, QColor color, int algType, bool phong, int typeOfFilling = 0);
	void drawLine3D(QVector3D start, QVector3D end, QColor color, int algType);
	void initHlbkaPixelov() {
		int rozmer = 500;
		hlbkaPixelov = std::vector<std::vector<double>>(rozmer, std::vector<double>(rozmer));

		for (int i = 0; i < rozmer; ++i) {
			for (int j = 0; j < rozmer; ++j) {
				hlbkaPixelov[i][j] = -DBL_MAX;
			}
		}
	}
	std::vector<QVector3D> trianglePoints;
	void phongModel(bool phong, int typeOfFilling, bool chcemVyfarbit, bool tocitSvetlom);
	QColor vratFarbu(QVector3D bod);
	double zhenit, azhimut;
	QVector3D farbaLuca,zdrojSvetla, ambient, farbySceny, odraz, difuzia, originalLightSource;
	double ostrost, poziciaKamery;

	QColor fillColor_() { return fillColor; };
	void setFillColor(QColor color) { fillColor = color; };

	QColor fillColor2_() { return fillColor2; };
	void setFillColor2(QColor color) { fillColor2 = color; };

	QColor fillColor3_() { return fillColor3; };
	void setFillColor3(QColor color) { fillColor3 = color; };

	int typeOfFilling() { return typeOffilling_; };
	void set_typeOfFilling(int type) { typeOffilling_ = type; };

	std::vector<QPoint>& polygon_points() { return polygon_points_; };
	void addPolygonPoint(QPoint point) { polygon_points_.push_back(point); };
	void clearPolygon() { polygon_points_.clear(); };

	std::vector<QPoint>& curvePoints() { return curvePoints_; };
	void addCurvePoint(QPoint point) { curvePoints_.push_back(point); };
	void clearCurve() { curvePoints_.clear(); };

	std::vector<double> smernice() { return smernice_; };
	void addSmernica(double s) { smernice_.push_back(s); };
	void clearSmernice() { smernice_.clear(); };
	void setSmernica(double s, int i) { smernice_[i] = s; };
	double getSmernica(int i) { return smernice_[i]; };
	std::vector<QPoint>& line_points() { return line_points_; };
	void addLinePoint(QPoint point) { line_points_.push_back(point); };
	void clearLine() { line_points_.clear(); };
	void setLinePoint(QPoint point, int index) { line_points_.at(index) = point; };

	std::vector<QPoint>& ellipseRadius() { return ellipseRadius_; };
	void addEllipsePoint(QPoint point) { ellipseRadius_.push_back(point); };
	void clearEllipse() { ellipseRadius_.clear(); };

	ViewerWidget(QSize imgSize, QWidget* parent = Q_NULLPTR);
	~ViewerWidget();
	void resizeWidget(QSize size);

	//Image functions
	bool setImage(const QImage& inputImg);
	QImage* getImage() { return img; };
	bool isEmpty();
	bool changeSize(int width, int height);

	void setPixel(int x, int y, uchar r, uchar g, uchar b, uchar a = 255);
	void setPixel(int x, int y, double valR, double valG, double valB, double valA = 1.);
	void setPixel(int x, int y, const QColor& color);
	bool isInside(int x, int y) { return (x >= 0 && y >= 0 && x < img->width() && y < img->height()) ? true : false; }

	//Draw functions
	void drawLine(QPoint start, QPoint end, QColor color, int algType = 0);
	void drawLineWithLengthAndSlope(QPoint startPoint, int length, double slope, QColor color, int algType);
	void setDrawLineBegin(QPoint begin) { drawLineBegin = begin; }
	QPoint getDrawLineBegin() { return drawLineBegin; }
	void setDrawLineActivated(bool state) { drawLineActivated = state; }
	bool getDrawLineActivated() { return drawLineActivated; }

	void drawCircle(QPoint start, QPoint end, QColor color);

	void drawEllipse(QPoint center, QPoint point1, QPoint point2, QColor color);
	void drawSymmetricPoints(int cx, int cy, int x, int y);

	void drawPolygon(std::vector<QPoint> points, QColor color, int algType, bool vyfarbit, QColor fillColor = Qt::transparent);
	void fillTriangle(std::vector<QPoint> points, QColor color, int algType);
	void fill(QPoint point1, QPoint point2, QPoint point3, QColor color);

	void setMoveBegin(QPoint begin) { moveBegin = begin; }
	QPoint getMoveBegin() { return moveBegin; }

	void posuvanie(std::vector<QPoint>& points, QColor color, QPoint offset, int algType, bool vyfarbit, QColor fillColor = Qt::transparent);
	void otacanie(std::vector<QPoint>& points, QColor color, double angle, int algType, bool vyfarbit, QColor fillColor = Qt::transparent);
	void skalovanie(std::vector<QPoint>& points, QColor color, double scaleX, double scaleY, int algType, bool vyfarbit, QColor fillColor = Qt::transparent);
	void osova_sumernost(std::vector<QPoint>& points, QColor color, int algType, bool vyfarbit, QColor fillColor = Qt::transparent);
	void skosenie(std::vector<QPoint>& points, QColor color, double koeficientSkosenia, int algType, bool vyfarbit, QColor fillColor = Qt::transparent);
	std::vector<QPoint> orezavanie(std::vector<QPoint> points, QColor color);
	std::vector<QPoint> clipLineByPolygon(std::vector<QPoint> points, QColor color);
	void fillPolygon(std::vector<QPoint> points, QColor color, double zValue = 0);


	void hermit(QColor color, int algType);
	void bezier(QColor color, int algType);
	void coonsov(QColor color, int algType);
	//Get/Set functions
	uchar* getData() { return data; }
	void setDataPtr() { data = img->bits(); }
	void setPainter() { painter = new QPainter(img); }

	int getImgWidth() { return img->width(); };
	int getImgHeight() { return img->height(); };

	void clear();

	//////////////////////3D GFAFIKA//////////////////////
	void premietanie(bool chcemVyfarbit);
	void pohladovaSustava(double zenit, double azimut, bool chcemStredove, double vzdialenost);
	void nove3DSuradnice(bool chcemStredove, double vzdialenost);

public slots:
	void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
};