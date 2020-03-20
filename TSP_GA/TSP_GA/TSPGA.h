#ifndef __TSPGA_H__
#define __TSPGA_H__



#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Progress.H>

#include <ctime>

#include "threads.h"
Fl_Thread GA_thread;

#include "Graph.h"

// UI Config
//
#define __Win_W 1280		// main window initial width
#define __Win_H 720			// main window initial height
#define __Graph_W0 50		// main window initial x-pos
#define __Graph_H0 50		// main window initial y-pos
#define __Graph_W 900		// graph window initial width
#define __Graph_H 600		// graph window initial height
#define __Ctrl_Col 1080

#define __INF_LEN_TOUR__ 9999999	// infinite tour length

int __vtxRadius = 5;		// city radius
Fl_Color __vtxColor = FL_RED;		// color of cities
Fl_Color __bestPathColor = FL_GREEN;	// color of best tour

int __Ctrl_row = 50;		// initial row position of UI elements
int __Ctrl_row_delta = 25;	// delta row position arrangments of UI elements


// Controller Widgets
Fl_Slider *sliders[4];					// slider arrays
Fl_Slider *verticesCountSlider;			//cities count slider
Fl_Slider *generationIterSlider;		// generation iteration count slider
Fl_Slider *populationCountSlider;		// choromosomes count slider
Fl_Slider *mutationPercentSlider;		// mutation percentage slider
Fl_Button *graphGenButton;				// generate graph button
Fl_Button *computeTSPButton;			// computTSP button
Fl_Button *StopComputingButton;			// stop computing button
Fl_Browser *infoBarBrowser;				// info box widget
Fl_Progress *progressBar;				// progress bar widget


// __DRAW_VID__ is defined for drawing city id
// by default if not defined
//
//#define __DRAW_VID__
#ifdef __DRAW_VID__
Fl_Box **__vtxIdLabels;		// city id labels
#endif

int __gVtxCount;				// cities count
int __generationIter;			// generation itertion count
int __toursPopulationCount;		// population (chromosomes) count in each generation
float __mutationProb;			// mutation percentage

bool __stopComputing = false;	// stop condition flag


// main window widget
//
class MainWindow : public Fl_Double_Window {
	void draw();		// must be overiden for suitable redraw
public:
	MainWindow(int w, int h, const char *l = 0)
		: Fl_Double_Window(w, h, l) {}
} *mainWindow;

// redraw all widgets
//
void MainWindow::draw() {
	Fl_Double_Window::draw();
	graphGenButton->redraw();
	computeTSPButton->redraw();
	infoBarBrowser->redraw();
	progressBar->redraw();
}


// Graph Window definition
//
class GraphWindow : public Fl_Double_Window {
	void draw();		// must be overriden
public:
	GraphWindow(int x, int y, int w, int h, const char *l = 0)
		: Fl_Double_Window(x, y, w, h, l) {}
}*graphWindow;


// override draw method to display Graph
//
void GraphWindow::draw() {
	// force to draw
	//
	Fl_Double_Window::draw();

	// draw the cities
	//
	if (__GVerices)  {
		for (int i = 0; i < __gVtxCount; i++) {
			fl_color(__vtxColor);
			int x = __GVerices[i]._x * w() / __Graph_W;
			int y = __GVerices[i]._y * h() / __Graph_H;
			fl_circle(x, y, __vtxRadius);
		}
	}

	// draw the best path
	if (__bestTour) {
		fl_color(__bestPathColor);
		fl_line_style(FL_DASH, 3);
		GEdgeVertex *path = __bestTour->_path;
		for (int j = 0; j < __gVtxCount; j++) {
			int x = path[j]._x * w() / __Graph_W;
			int y = path[j]._y * h() / __Graph_H;
			int x2 = path[(j + 1) % __gVtxCount]._x * w() / __Graph_W;
			int y2 = path[(j + 1) % __gVtxCount]._y * h() / __Graph_H;
			fl_line(x, y, x2, y2);
		}
	}
}



// function to write information into the info browser widget
//
void writeInfo(const GPath *tour) {
	char infoStr[20];
	sprintf_s(infoStr, 20, "%.2f", tour->_length);
	infoBarBrowser->add(infoStr);
	infoBarBrowser->bottomline(infoBarBrowser->size());
}


// function to deactivate buttons on UI
//
void deactivateUI() {
	graphGenButton->deactivate();
	computeTSPButton->deactivate();
	mainWindow->redraw();
}

// function to activate buttons on UI
//
void activateUI() {
	graphGenButton->activate();
	computeTSPButton->activate();
	infoBarBrowser->add("computation complete");
	infoBarBrowser->add("----------------------------");
	infoBarBrowser->bottomline(infoBarBrowser->size());
	mainWindow->redraw();
}

// function to write header information before starting GA 
//
void writeHeaderInfo() {
	infoBarBrowser->add("TSP_GA_CX strting...");
	char geneStr[32];
	sprintf_s(geneStr, 32, "%s %d", "generations:", __generationIter);
	infoBarBrowser->add(geneStr);
	char chromeStr[32];
	sprintf_s(chromeStr, 32, "%s %d", "chromosomes:", __toursPopulationCount);
	infoBarBrowser->add(chromeStr);
	char mutStr[32];
	sprintf_s(mutStr, 32, "%s %.3f", "mutation:", __mutationProb);
	infoBarBrowser->add(mutStr);
	infoBarBrowser->bottomline(infoBarBrowser->size());
}



// call back function to generate random cities
//
void generateCities(Fl_Widget *, void *) {
	// check if there exist previously generated cities, 
	// garbage collect all cities
	//
	if (__GVerices) {
		delete[] __GVerices;
		__GVerices = NULL;

#ifdef __DRAW_VID__
		for (int i = 0; i < __gVtxCount; i++)	{ delete __vtxIdLabels[i]; __vtxIdLabels[i] = NULL; }
		delete[] __vtxIdLabels;
		__vtxIdLabels = NULL;
#endif
	}
	// garbage collect all previously generated tours
	//
	if (__toursPopulationList) {
		for (int i = 0; i < __gVtxCount; i++) {
			delete[] __toursPopulationList[i]->_path;
			__toursPopulationList[i]->_path = NULL;
		}
		delete[] __toursPopulationList;
		__toursPopulationList = NULL;
	}
	// delete the current best tour if it exists
	//
	if (__bestTour) {
		delete __bestTour;
		__bestTour = NULL;
	}
	// set vertices count variable fro UI
	//
	__gVtxCount = (int)verticesCountSlider->value();
	// check if vertices count is not set to zero
	//
	if (__gVtxCount == 0)
		return;
	// create new vertices array
	//
	__GVerices = new GEdgeVertex[__gVtxCount];
#ifdef __DRAW_VID__
	__vtxIdLabels = new Fl_Box *[__gVtxCount];	// create vId labels if __DRAW_VID__ is defined
#endif
	// set vertices positions randomly
	//
	for (int i = 0; i < __gVtxCount; i++) {
		__GVerices[i]._vid = i;
		__GVerices[i]._x = rand() % __Graph_W;
		__GVerices[i]._y = rand() % __Graph_H;
#ifdef __DRAW_VID__
		// draw vId labels if __DRAW_VID__ is defined
		//
		char vIdStr[4];
		sprintf_s(vIdStr, 4, "%d", i);
		fl_color(FL_WHITE);
		__vtxIdLabels[i] = new Fl_Box(__GVerices[i]._x, __GVerices[i]._y, 50, 50, "1");
#endif
	}
	// write info
	//
	infoBarBrowser->add("------------------------------");
	char infoStr[26];
	sprintf_s(infoStr, 26, "%d %s", __gVtxCount, "cities generated..");
	infoBarBrowser->add(infoStr);
	infoBarBrowser->bottomline(infoBarBrowser->size());
	graphWindow->redraw();
}



// function to check if there exist given index in the given list,
// return true if so, otherwise return false
//
bool isInList(int *list, int indx, int count) {
	for (int i = 0; i < count; i++) {
		if (list[i] == indx)
			return true;
	}
	return false;
}



// function to generated random permutaion list of indicies,
// reutrn a permutation list of integer array
//
int *permutaionList() {
	int *pList = new int[__gVtxCount];
	for (int i = 0; i < __gVtxCount; i++) {
		int r = (int)rand() % __gVtxCount;
		while (isInList(pList, r, i)) {
			r = (int)rand() % __gVtxCount;
		}
		pList[i] = r;
	}
	return pList;
}



// function to generate a random path, tour indeed, based on the random permutation integer list,
// return gPath as a random path
//
GPath *generateRandPath() {
	GPath *gPath = new GPath;
	gPath->_path = new GEdgeVertex[__gVtxCount];
	int *pList = permutaionList();
	for (int i = 0; i < __gVtxCount; i++)
		gPath->_path[i] = __GVerices[pList[i]];
	for (int i = 0; i < __gVtxCount; i++)
		gPath->updateLength();
	return gPath;
}



// function to generate the initial population, random chromosomes,
// set __toursPopulationList variable
//
void initiatePopulation() {
	srand(clock());
	if (__toursPopulationList) {
		for (int i = 0; i < __toursPopulationCount; i++) {
			delete[] __toursPopulationList[i]->_path;
			__toursPopulationList[i]->_path = NULL;
		}
		delete[] __toursPopulationList;
		__toursPopulationList = NULL;
	}
	__toursPopulationCount = (int)populationCountSlider->value();
	__toursPopulationList = new GPath *[__toursPopulationCount];
	for (int i = 0; i < __toursPopulationCount; i++) {
		__toursPopulationList[i] = generateRandPath();
	}
	// write info
	//
	char infoStr[32];
	sprintf_s(infoStr, 32, "%d %s", __toursPopulationCount, "chromosomes made");
	infoBarBrowser->add(infoStr);
	infoBarBrowser->bottomline(infoBarBrowser->size());
	mainWindow->redraw();
}



// function to sort the tours in the current population based on fitnesses, i.e. the length of the tour,
// resort __toursPopulationList based on insertion sort method
//
void sortTours() {
	GPath *tour = NULL;
	int j;
	for (int i = 1; i < __toursPopulationCount; i++) {
		tour = __toursPopulationList[i];
		j = i - 1;
		while (j >= 0 && tour->_length < __toursPopulationList[j]->_length) {
			__toursPopulationList[j + 1] = __toursPopulationList[j--];
		}
		__toursPopulationList[j + 1] = tour;
	}
}


void merge(GPath **A, unsigned l, unsigned m, unsigned u) {
	GPath **B = new GPath *[u - l + 1];
	unsigned h = l;
	unsigned k = m + 1;
	unsigned j = 0;
	while (h <= m  &&  k <= u) {
		if (A[h]->_length < A[k]->_length)
			B[j++] = A[h++];
		else
			B[j++] = A[k++];
	}	
	if (h > m)	for (unsigned r = k; r <= u; B[j++] = A[r++]);
	else		for (unsigned r = h; r <= m; B[j++] = A[r++]);
	for (unsigned r = l; r <= u; A[r++] = B[r - l]);
	delete []B;
}
void mergeSort(GPath **A, unsigned l, unsigned u) {
	if (l < u){
		unsigned m = (l + u) / 2;
		mergeSort(A, l, m);
		mergeSort(A, m + 1, u);
		merge(A, l, m, u);
	}
}


// function to select, return true, based on the given probability value, between 0.0 and 1.0,
// return false otherswise
//
bool selectByProbability(float p) {
	float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	if (r < p)
		return true;
	else
		return false;
}



// search the given input in the given list to find the index associated to
// return index if it exists, return -1 otherswise, if every thing went right this always return an index
//
int searchIdx(const GPath *path, const int x) {
	for (int i = 0; i < __gVtxCount; i++) {
		if (path->_path[i]._vid == x) {
			return i;
		}
	}
	return -1;
}



// function to operation mutation on the given chromosome,
//
void mutate(GPath *tour) {
	// select to random indicex, r1 and r2
	//
	int r1 = rand() % __gVtxCount;
	int r2 = rand() % __gVtxCount;
	// guarantee the r1 and r2 are not equal
	//
	while (r1 == r2)
		r2 = rand() % __gVtxCount;
	// swap the two randomly selected genes
	//
	GEdgeVertex vtx = tour->_path[r1];
	tour->_path[r1] = tour->_path[r2];
	tour->_path[r2] = vtx;
	// update the mutated chromosome length
	//
	tour->updateLength();
}



// function to select survivors based on the rank-based selection ,method by the probability of (i/n)
// return the index of the last survivor + 1, i.e. the index of the first looser
//
int selectSurvivors() {
	// first sort the population, so the best fitted choromosomes are on the top
	//
	sortTours();
	//mergeSort(__toursPopulationList, 0, __toursPopulationCount - 1);
	int k = 0;
	// iterate through all the chromosomes to select the survivors based on the rank-based probability, i.e. (i/n)
	//
	for (int i = 0; i < __toursPopulationCount; i++) {
		// if this chromosome survived, pass on it and just increment k by one
		//
		if (selectByProbability(1.0f - (float)i / __toursPopulationCount)) {
			k++;
		}
		// if this choromosome failed, set its tour length to infinity
		//
		else {
			__toursPopulationList[i]->_length = __INF_LEN_TOUR__;
		}
	}
	// resort the population list to bring the survivors up and the loosers down
	//
	sortTours();
	//mergeSort(__toursPopulationList, 0, __toursPopulationCount - 1);
//	SYSTEMTIME st;
//	GetSystemTime(&st);
//	cout << st.wSecond << endl;
	// now k is the index of the last survivor + 1, i.e. the index of the first looser
	//
	return k;		
}



// function to Cycle crossover, CX, of two parents,
// return the two children if returnTwo if set to true, otherwise return one child,
//
void CX_crossover(const GPath *parent1, const GPath *parent2, GPath **child1, GPath **child2, bool returnTwo) {
	int k, h = 0;
	// iterate through the parents genes to check the equality of their genes
	//
	while (parent1->_path[h]._vid == parent2->_path[h++]._vid) {
		// check if h is not overflow
		// if so, break the iteration
		//
		if (h >= __gVtxCount) {
			h--;	// now h is the index of the first inequal gene of the parent chromosomes
			break;
		}
	}
	// generate child 1
	//
	*child1 = new GPath();
	(*child1)->_path = new GEdgeVertex[__gVtxCount];
	(*child1)->_path[h] = parent1->_path[h];
	// set k to h, so CX starts from h-th index, the first inequal gene
	//
	k = h;	
	// start mapping from k, the first inequal gene
	//
	k = searchIdx(parent1, parent2->_path[k]._vid);
	// start iteration with the condidtion of cycle check,
	// and assign the genes to the child1 by cyclically mapping the common city Ids
	//
	while (k != h) {
		(*child1)->_path[k] = parent1->_path[k];
		k = searchIdx(parent1, parent2->_path[k]._vid);
	}
	// fill blank gens based on the id of the cities, if it is -1 so it is empty, otherwise is set already
	//
	for (int i = 0; i < __gVtxCount; i++) {
		if ((*child1)->_path[i]._vid == -1)
			(*child1)->_path[i] = parent2->_path[i];
	}
	// update child1 tour length
	//
	(*child1)->updateLength();
	// check if returnTwo is set to true to generate the second child
	//
	if (returnTwo) {
		// generate child 2
		//
		*child2 = new GPath();
		(*child2)->_path = new GEdgeVertex[__gVtxCount];
		(*child2)->_path[h] = parent2->_path[h];
		k = h;
		k = searchIdx(parent2, parent1->_path[k]._vid);
		while (k != h) {
			(*child2)->_path[k] = parent2->_path[k];
			k = searchIdx(parent2, parent1->_path[k]._vid);
		}
		// fill blank gens
		//
		for (int i = 0; i < __gVtxCount; i++) {
			if ((*child2)->_path[i]._vid == -1)
				(*child2)->_path[i] = parent1->_path[i];
		}
		// update child2 tour length
		//
		(*child2)->updateLength();
	}
}



// core function to compute TSP using GA based on CX crossover and rank-based selection
//
void *TSP_GA_CX(void *) {
	// check if cities are generated
	//
	if (!__GVerices)
		return 0;
	// deactivate the UI, set stop condidtion to false, update seed, read gen Iter and mutation from UI
	//
	deactivateUI();
	__stopComputing = false;
	srand(clock());
	__generationIter = (int)generationIterSlider->value();
	__mutationProb = (float)mutationPercentSlider->value() / 100.0f;
	// initiate random population 
	//
	initiatePopulation();
	// set the default best tour to the first initial chromosome
	//
	__bestTour = __toursPopulationList[0];
	// write header info and update drawing
	//
	writeHeaderInfo();
	graphWindow->redraw();
	// select the first list of survivors
	//
	int k = selectSurvivors();
	// iterates to generate the generations of populations
	//
	for (int i = 0; i < __generationIter; i++) {
		// iterate through the current population list from k-th index, i.e. the index of the first looser, to replace the newly made children
		//
		while (k < __toursPopulationCount) {
			// generate random p1 and p2
			//
			int p1 = rand() % k;
			int p2 = rand() % k;
			// guarantee p1 and p2 are not equal
			//
			while (p1 == p2)
				p2 = rand() % k;
			// delete the k-th chromosome and make the child1 ready to be replaced with
			//
			delete[] __toursPopulationList[k]->_path;
			delete __toursPopulationList[k];
			GPath *child1 = NULL;
			k++;
			// after incrementing k, check if k is still inside the population
			// if so, two childern generated and replaced
			//
			if (k < __toursPopulationCount) {
				// delete the (k+1)-th chromosome and make the child1 ready to be replaced with 
				// notice the k was already incemented by 1
				//
				delete[] __toursPopulationList[k]->_path;
				delete __toursPopulationList[k];
				GPath *child2 = NULL;
				// operate CX cross over on p1 and p2 chromosomes to generate child1 and child2
				//
				CX_crossover(__toursPopulationList[p1], __toursPopulationList[p2], &child1, &child2, true);
				// now child1 and child2 are made
				// mutate the child1 based on the given probability and replace it with the old one
				//
				if (__mutationProb != 0.0f  &&  selectByProbability(__mutationProb))
					mutate(child1);
				__toursPopulationList[k - 1] = child1;
				// mutate the child2 based on the given probability and replace it with the old one
				//
				if (__mutationProb != 0.0f  &&  selectByProbability(__mutationProb))
					mutate(child2);
				__toursPopulationList[k] = child2;
				k++;
			}
			// (k < __toursPopulationCount), otherwise k is out of the population,
			// so just one child is made
			//
			else {
				// operate CX cross over on p1 and p2 chromosomes to generate child1
				//
				CX_crossover(__toursPopulationList[p1], __toursPopulationList[p2], &child1, NULL, false);
				// now child1 is made
				// mutate the child1 based on the given probability and replace it with the old one
				//
				if (__mutationProb != 0.0f  &&  selectByProbability(__mutationProb))
					mutate(child1);
				__toursPopulationList[k - 1] = child1;
			}
			// check if the stop computing button is pressed
			// if so, terminate the computation
			//
			if (__stopComputing) {
				return 0;
			}
		}
		// select new survivors from the newly set population
		//
		k = selectSurvivors();
		// set the best solution by checking if there is the better best tour
		// as the best tour in each generation is the first tour after sorting, so the best tour is compared with the first tour of each generation
		//
		if (__toursPopulationList[0]->_length < __bestTour->_length) {
			__bestTour = __toursPopulationList[0];
			writeInfo(__bestTour);
			graphWindow->redraw();
		}
		// update the progress bar
		//
		progressBar->value(float(i) / __generationIter);
	}
	// activate the UI after terminating the iteration
	//
	progressBar->value(1.0f);
	activateUI();
	return 0;
}



// call back function to run TSP_GA_CX
//
void TSP_GA_Cx(Fl_Widget *, void *) {
	// create a thread for TSP_GA_CX procedure to prevent the graph window from being locked.
	// so the graph window and other UI elements become updated interactively while computing
	//
	fl_create_thread(GA_thread, TSP_GA_CX, (void *)0);
}


// call back function to stop computing by pressing Stop Computing
// just set __stopComputing to true and activate UI elements
//
void stopComputing(Fl_Widget *, void *) {
	__stopComputing = true;
	activateUI();
}


//------------------------------------------


// function to make the main window
//
void makeMainWindow(const char *) {
	mainWindow = new MainWindow(__Win_W, __Win_H, "TSP_GA");
	sliders[0] = verticesCountSlider = new Fl_Value_Slider(__Ctrl_Col, __Ctrl_row, 180, 20, "Cities");	__Ctrl_row += __Ctrl_row_delta;
	verticesCountSlider->bounds(2, 50);
	verticesCountSlider->value(2);

	graphGenButton = new Fl_Button(__Ctrl_Col, __Ctrl_row, 180, 20, "Generate Cities");	__Ctrl_row += __Ctrl_row_delta + 25;
	graphGenButton->callback(generateCities);

	sliders[1] = generationIterSlider = new Fl_Value_Slider(__Ctrl_Col, __Ctrl_row, 180, 20, "Generaton");	__Ctrl_row += __Ctrl_row_delta;
	generationIterSlider->bounds(1, 5000);
	generationIterSlider->value(1);

	sliders[2] = populationCountSlider = new Fl_Value_Slider(__Ctrl_Col, __Ctrl_row, 180, 20, "Chromosomes");	__Ctrl_row += __Ctrl_row_delta;
	populationCountSlider->bounds(2, 80000);
	populationCountSlider->value(2);

	sliders[3] = mutationPercentSlider = new Fl_Value_Slider(__Ctrl_Col, __Ctrl_row, 180, 20, "Mutation%");	__Ctrl_row += __Ctrl_row_delta + 10;
	mutationPercentSlider->bounds(0, 100);

	computeTSPButton = new Fl_Button(__Ctrl_Col, __Ctrl_row, 180, 20, "Compute TSP");		__Ctrl_row += __Ctrl_row_delta + 10;
	computeTSPButton->callback(TSP_GA_Cx);

	infoBarBrowser = new Fl_Browser(__Ctrl_Col, __Ctrl_row, 180, 300);		__Ctrl_row += __Ctrl_row_delta + 290;

	progressBar = new Fl_Progress(__Ctrl_Col, __Ctrl_row, 180, 20);		__Ctrl_row += __Ctrl_row_delta + 20;
	progressBar->minimum(0.0);
	progressBar->maximum(1.0);
	progressBar->color2(FL_BLUE);

	StopComputingButton = new Fl_Button(__Ctrl_Col, __Ctrl_row, 180, 20, "Stop Computing");		__Ctrl_row += __Ctrl_row_delta + 20;
	StopComputingButton->callback(stopComputing);

	for (int i = 0; i<4; i++) {
		sliders[i]->type(1);
		sliders[i]->align(FL_ALIGN_LEFT);
		if (i == 3)
			sliders[i]->step(0.5);
		else
			sliders[i]->step(1);
	}

	graphWindow = new GraphWindow(__Graph_W0, __Graph_H0, __Graph_W, __Graph_H);
	graphWindow->color(FL_BLACK);
	graphWindow->end();
	Fl_Box *b1 = new Fl_Box(590, 670, 80, 20, "TSP computing using GA, Crossover using CX, Selection using Rank-based strategy");
	b1->align(FL_ALIGN_LEFT);
	b1->labelcolor(FL_BLUE);
	new  Fl_Tile(600, 700, 400, 20);
	Fl_Box *b2 = new Fl_Box(300, 690, 80, 20, "Developed and Written by Hadi Sarraf");
	b2->align(FL_ALIGN_LEFT);
	b2->labelcolor(FL_BLUE);
	mainWindow->resizable(graphWindow);
	mainWindow->end();
}



#endif /*__TSPGA_H__*/