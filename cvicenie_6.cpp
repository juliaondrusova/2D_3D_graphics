#include <iostream>
#include <fstream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>

class W_hrana; // Deklarácia dopredu
class Vrchol {

	double x, y, z;
	W_hrana* hrana;

};
class Plocha {

	W_hrana* hrana;

};
class W_hrana {

	Vrchol* vrchol_pôvodu, * vrchol_cieľ;
	Plocha* ľavá_plocha, * pravá_plocha;
	W_hrana* ľavá_hrana_predchádzajúca, * ľavá_hrana_nasledujúca;
	W_hrana* pravá_hrana_predchádzajúca, * pravá_hrana_nasledujúca;

};


struct Point
{
	double x, y, z;
	int poradie;
};




void vytvorSferu(std::ofstream& outfile, int polomer, int pocetPoludnikov, int pocetRovnobeziek)
{
	pocetRovnobeziek--;
	double uholRovnobeziek = M_PI / (double)(pocetRovnobeziek);
	double uholPoludnikov = (2 * M_PI) / (double)(pocetPoludnikov);

	int num_points = (pocetRovnobeziek - 1) * pocetPoludnikov + 2;
	int num_lines = pocetPoludnikov + (pocetRovnobeziek - 1) * pocetPoludnikov * 2;
	int num_polygons = 2 * (pocetRovnobeziek - 1) * pocetPoludnikov;

	outfile << "# vtk DataFile Version 3.0\nvtk output\nASCII\nDATASET POLYDATA\n";
	outfile << "POINTS " << num_points << " float\n";

	outfile << "0 0 " << polomer << "\n";
	for (int i = 1; i < pocetRovnobeziek; i++)
	{
		double theta1 = i * uholRovnobeziek;

		for (int j = 0; j < pocetPoludnikov; j++)
		{
			double phi1 = j * uholPoludnikov;

			double x = polomer * sin(theta1) * cos(phi1);
			double y = polomer * sin(theta1) * sin(phi1);
			double z = polomer * cos(theta1);

			outfile << x << " " << y << " " << z << "\n";
		}
	}
	outfile << "0 0 " << -polomer << "\n";

	outfile << "LINES " << num_lines << " " << num_lines * 3 << "\n";

	for (int j = 0; j < pocetPoludnikov; ++j) {
		outfile << "2 0 " << j + 1 << "\n";
	}

	for (int i = 0; i < pocetRovnobeziek - 1; ++i) {
		int start_index = 1 + i * pocetPoludnikov;
		for (int j = 0; j < pocetPoludnikov; ++j) {
			int next_j = (j + 1) % pocetPoludnikov;
			outfile << "2 " << start_index + j << " " << start_index + next_j << "\n";
			if (i < pocetRovnobeziek - 2)
				outfile << "2 " << start_index + j << " " << start_index + pocetPoludnikov + j << "\n";
		}
	}


	int last_parallel_start = (pocetRovnobeziek - 2) * pocetPoludnikov + 1;
	for (int j = 0; j < pocetPoludnikov; ++j) {
		outfile << "2 " << last_parallel_start + j << " " << (pocetRovnobeziek - 1) * pocetPoludnikov + 1 << "\n";
	}

	outfile << "POLYGONS " << num_polygons << " " << 4 * num_polygons << "\n";


	for (int j = 0; j < pocetPoludnikov - 1; ++j) {
		outfile << "3 0 " << j + 1 << " " << j + 2 << "\n";
	}
	outfile << "3 0 1 " << pocetPoludnikov << "\n";

	for (int i = 1; i < pocetRovnobeziek - 1; ++i) {
		int current_row_start = 1 + (i - 1) * pocetPoludnikov;
		int next_row_start = 1 + i * pocetPoludnikov;
		for (int j = 0; j < pocetPoludnikov; ++j) {
			int next_j = (j + 1) % pocetPoludnikov;
			outfile << "3 " << current_row_start + j << " " << next_row_start + j << " " << current_row_start + next_j << "\n";
			outfile << "3 " << current_row_start + next_j << " " << next_row_start + j << " " << next_row_start + next_j << "\n";
		}
	}

	for (int j = 0; j < pocetPoludnikov - 1; ++j) {
		outfile << "3 " << last_parallel_start + j << " " << last_parallel_start + j + 1 << " " << (pocetRovnobeziek - 1) * pocetPoludnikov + 1 << "\n";
	}
	outfile << "3 " << last_parallel_start + pocetPoludnikov - 1 << " " << last_parallel_start << " " << (pocetRovnobeziek - 1) * pocetPoludnikov + 1 << "\n";

}



/*
void vytvorKocku(std::ofstream& outfile, int dlzkaHrany)
{
	outfile << "# vtk DataFile Version 3.0\nvtk output\nASCII\nDATASET POLYDATA\nPOINTS 8 float\n";

	int centerX = 0;
	int centerY = 0;
	int centerZ = 0;

	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			for (int k = 0; k < 2; ++k)
			{
				outfile << (i * dlzkaHrany) + centerX << " " << (j * dlzkaHrany) + centerY << " " << (k * dlzkaHrany) + centerZ << "\n";
			}
		}
	}
	outfile << "LINES 12 36\n";
	outfile << "2 0 1\n";
	outfile << "2 1 3\n";
	outfile << "2 3 2\n";
	outfile << "2 2 0\n";
	outfile << "2 4 5\n";
	outfile << "2 5 7\n";
	outfile << "2 7 6\n";
	outfile << "2 6 4\n";
	outfile << "2 0 4\n";
	outfile << "2 1 5\n";
	outfile << "2 3 7\n";
	outfile << "2 2 6\n";

	// Write the polygons for the faces of the cube
	outfile << "POLYGONS 6 30\n";
	outfile << "4 0 1 3 2\n";
	outfile << "4 4 5 7 6\n";
	outfile << "4 0 1 5 4\n";
	outfile << "4 2 3 7 6\n";
	outfile << "4 0 2 6 4\n";
	outfile << "4 1 3 7 5\n";
}
*/

/*
void vytvorKocku(std::ofstream& outfile, int dlzkaHrany)
{
	outfile << "# vtk DataFile Version 3.0\nvtk output\nASCII\nDATASET POLYDATA\nPOINTS 8 float\n";

	int centerX = dlzkaHrany / 2;
	int centerY = dlzkaHrany / 2;
	int centerZ = dlzkaHrany / 2;

	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			for (int k = 0; k < 2; ++k)
			{
				// Adjust vertex positions to make the cube symmetric around (0, 0, 0)
				int x = (i == 0) ? -centerX : dlzkaHrany - centerX;
				int y = (j == 0) ? -centerY : dlzkaHrany - centerY;
				int z = (k == 0) ? -centerZ : dlzkaHrany - centerZ;
				outfile << x << " " << y << " " << z << "\n";
			}
		}
	}

	outfile << "LINES 12 36\n";
	outfile << "2 0 1\n";
	outfile << "2 1 3\n";
	outfile << "2 3 2\n";
	outfile << "2 2 0\n";
	outfile << "2 5 4\n";
	outfile << "2 4 6\n";
	outfile << "2 6 7\n";
	outfile << "2 7 5\n";
	outfile << "2 4 0\n";
	outfile << "2 1 5\n";
	outfile << "2 3 7\n";
	outfile << "2 2 6\n";

	// Write the polygons for the faces of the cube
	outfile << "POLYGONS 6 30\n";
	outfile << "4 0 1 3 2\n";
	outfile << "4 4 5 7 6\n";
	outfile << "4 0 1 5 4\n";
	outfile << "4 2 3 7 6\n";
	outfile << "4 0 2 6 4\n";
	outfile << "4 1 3 7 5\n";

}
*/


void vytvorKocku(std::ofstream& outfile, int dlzkaHrany)
{
	outfile << "# vtk DataFile Version 3.0\nvtk output\nASCII\nDATASET POLYDATA\nPOINTS 8 float\n";

	int centerX = dlzkaHrany / 2;
	int centerY = dlzkaHrany / 2;
	int centerZ = dlzkaHrany / 2;

	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			for (int k = 0; k < 2; ++k)
			{
				// Adjust vertex positions to make the cube symmetric around (0, 0, 0)
				int x = (i == 0) ? -centerX : dlzkaHrany - centerX;
				int y = (j == 0) ? -centerY : dlzkaHrany - centerY;
				int z = (k == 0) ? -centerZ : dlzkaHrany - centerZ;
				outfile << x << " " << y << " " << z << "\n";
			}
		}
	}

	outfile << "LINES 18 54\n"; // Change the count to 24 since we're adding 12 additional lines
	outfile << "2 0 1\n";
	outfile << "2 1 3\n";
	outfile << "2 3 2\n";
	outfile << "2 2 0\n";
	outfile << "2 5 4\n";
	outfile << "2 4 6\n";
	outfile << "2 6 7\n";
	outfile << "2 7 5\n";
	outfile << "2 4 0\n";
	outfile << "2 1 5\n";
	outfile << "2 3 7\n";
	outfile << "2 2 6\n";

	// Add diagonal lines for each face
	outfile << "2 0 3\n"; // Front face diagonal
	outfile << "2 4 7\n"; // Back face diagonal
	outfile << "2 0 5\n"; // Top face diagonal
	outfile << "2 2 7\n"; // Bottom face diagonal
	outfile << "2 1 7\n"; // Left face diagonal
	outfile << "2 0 6\n"; // Right face diagonal

	// Write the polygons for the faces of the cube
	outfile << "POLYGONS 12 48\n";
	outfile << "3 0 1 3\n"; // Front face
	outfile << "3 0 3 2\n";
	outfile << "3 4 5 7\n"; // Back face
	outfile << "3 4 7 6\n";
	outfile << "3 0 5 1\n"; // Top face
	outfile << "3 0 5 4\n";
	outfile << "3 2 7 3\n"; // Bottom face
	outfile << "3 2 7 6\n";
	outfile << "3 1 7 3\n"; // Left face
	outfile << "3 1 5 7\n";
	outfile << "3 0 6 2\n"; // Right face
	outfile << "3 0 6 4\n";
}


void vytvorSferu1(std::ofstream& outfile, double polomer, int pocetPoludnikov, int pocetRovnobeziek)
{
	pocetRovnobeziek--; //pridanim spodneho a vrchneho mi vzniknu 2 navyse, ale o jednu menej robim v cykle - takze len minus 1
	double uholRovnobeziek = M_PI / (double)pocetRovnobeziek;
	double uholPoludnikov = (2 * M_PI) / (double)(pocetPoludnikov);

	std::vector<std::vector<Point>> points(pocetRovnobeziek, std::vector<Point>(pocetPoludnikov));

	outfile << "# vtk DataFile Version 3.0\nvtk output\nASCII\nDATASET POLYDATA\n";
	outfile << "POINTS " << (pocetPoludnikov) * (pocetRovnobeziek - 1) + 2 << " float\n"; // -1 lebo v cykle ideme od i=1 a +2 lebo prvy a posledny bod

	outfile << "0 0 " << polomer << "\n";
	for (int i = 1; i < pocetRovnobeziek; i++) //zaciname od 1 -> pre i=0 a i=pocetRovnobeziek by boli vsetky j-te body aj tak rovnake
	{
		double teta = i * uholRovnobeziek;
		for (int j = 0; j < pocetPoludnikov; j++) //nemusim po j=pocetPoludnikov - su to rovnake body ako pre j=0
		{
			double fi = j * uholPoludnikov;
			double x = polomer * sin(teta) * cos(fi);
			double y = polomer * sin(teta) * sin(fi);
			double z = polomer * cos(teta);

			outfile << x << " " << y << " " << z << "\n";
			points[i - 1][j] = { x,y,z, (i - 1) * pocetPoludnikov + j + 1 };
		}
	}
	outfile << "0 0 " << -polomer << "\n";

//	outfile << "LINES " << pocetPoludnikov + (pocetRovnobeziek - 1) * pocetPoludnikov * 2 << " " << (pocetPoludnikov + (pocetRovnobeziek - 1) * pocetPoludnikov * 2) * 3 << "\n";
	outfile << "LINES " << 2 * pocetPoludnikov + (pocetRovnobeziek - 1) * pocetPoludnikov + (pocetRovnobeziek - 2) * pocetPoludnikov + (pocetRovnobeziek - 3) * pocetPoludnikov << " " << (2 * pocetPoludnikov + (pocetRovnobeziek - 1) * pocetPoludnikov + (pocetRovnobeziek - 2) * pocetPoludnikov + (pocetRovnobeziek - 3) * pocetPoludnikov) * 3 << "\n";

	//prvy bod potrebujem spojit s bodmi na prvej rovnobezke
	for (int i = 0; i < pocetPoludnikov; i++)
	{
		outfile << "2 0 " << points[0][i].poradie << "\n";
	}

	//posledny bod potrebujem spojit s bodmi na poslednej rovnobezke
	for (int i = 0; i < pocetPoludnikov; i++)
	{
		outfile << "2 " << points[pocetRovnobeziek - 1 - 1][pocetPoludnikov - 1].poradie + 1 << " " << points[pocetRovnobeziek - 1 - 1][i].poradie << "\n";
	}

	for (int i = 0; i < pocetRovnobeziek - 1; i++)
	{
		for (int j = 0; j < pocetPoludnikov; j++)
		{
			outfile << "2 " << points[i][j].poradie << " " << points[i][(j + 1) % pocetPoludnikov].poradie << "\n"; //spajam susedne body na jednej rovnobezke
			if (i < (pocetRovnobeziek - 2))
				outfile << "2 " << points[i][j].poradie << " " << points[(i + 1) % pocetRovnobeziek][j].poradie << "\n"; //spajam dva body na tom istom poludniku
			if (i < (pocetRovnobeziek - 3))
				outfile << "2 " << points[i][(j + 1) % pocetPoludnikov].poradie << " " << points[(i + 1) % pocetRovnobeziek][j].poradie << "\n"; //spajam bod na poludniku s bodom o poludnik vyssie a doprava


		}
	}

	outfile << "POLYGONS " << 2 * (pocetRovnobeziek - 1) * pocetPoludnikov << " " << 4 * (2 * (pocetRovnobeziek - 1) * pocetPoludnikov) << "\n";

	//outfile << "POLYGONS " << 2*pocetPoludnikov + ((pocetRovnobeziek - 2) * (pocetPoludnikov)) << " " << 4 * 2*pocetPoludnikov + (5 * (pocetRovnobeziek - 2) * (pocetPoludnikov)) << "\n";
	//vytvaram trojuholnikove plosky medzi prvym bodom a bodmi na prvej rovnobezke
	for (int i = 0; i < pocetPoludnikov; i++)
	{
		outfile << "3 0 " << points[0][i].poradie << " " << points[0][(i + 1) % pocetPoludnikov].poradie << "\n";
	}

	for (int i = 0; i < pocetPoludnikov; i++)
	{
		outfile << "3 " << points[pocetRovnobeziek - 1 - 1][pocetPoludnikov - 1].poradie + 1 << " " << points[pocetRovnobeziek - 1 - 1][i].poradie << " " << points[pocetRovnobeziek - 1 - 1][(i + 1) % pocetPoludnikov].poradie << "\n";
	}

	//outfile << "POLYGONS " << (pocetRovnobeziek - 2) * (pocetPoludnikov) << " " << 5 * (pocetRovnobeziek - 2) * (pocetPoludnikov) << "\n";

	for (int i = 0; i < pocetRovnobeziek - 2; i++)
	{
		for (int j = 0; j < pocetPoludnikov; j++)
		{

			//outfile << "4 " << points[i][j].poradie << " " <<  points[(i + 1)][j].poradie << " " << points[(i + 1)][(j + 1) % pocetPoludnikov].poradie << " " << points[i][(j + 1) % pocetPoludnikov].poradie << "\n";

			outfile << "3 " << points[i][j].poradie << " " << points[i + 1][j].poradie << " " << points[i][(j + 1) % pocetPoludnikov].poradie << "\n";
			outfile << "3 " << points[i][(j + 1) % pocetPoludnikov].poradie << " " << points[i + 1][j].poradie << " " << points[i + 1][(j + 1) % pocetPoludnikov].poradie << "\n";

		}
	}

}

int main()
{
	std::ofstream outfile("cube.vtk");
	if (outfile.is_open())
	{
		vytvorKocku(outfile, 200); // Assuming edge length of 1 for simplicity
		outfile.close();
		std::cout << "VTK file created successfully." << std::endl;
	}
	else
	{
		std::cerr << "Unable to open file." << std::endl;
		return 1;
	}


	std::ofstream outfile1("sphere.vtk");
	if (outfile1.is_open())
	{
		vytvorSferu1(outfile1, 200, 20, 20); // Assuming edge length of 1 for simplicity
		outfile1.close();
		std::cout << "VTK file created successfully." << std::endl;
	}
	else
	{
		std::cerr << "Unable to open file." << std::endl;
		return 1;
	}

	return 0;
}
