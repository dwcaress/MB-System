#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "vtkDataSet.h"
#include "vtkSetGet.h"
#include "vtkCallbackCommand.h"
#include "vtk_netcdf.h"
#include "GmtGridReader.h"

#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()



GmtGridReader::GmtGridReader() :
  fileName_(nullptr) {
  
  gridPoints_ = vtkSmartPointer<vtkPoints>::New();
  gridPoints_->SetDataTypeToFloat();
  gridPolygons_ = vtkSmartPointer<vtkCellArray>::New();  

  this->SetNumberOfInputPorts(0);

  VTK_CREATE(vtkCallbackCommand, cbc);
  cbc->SetCallback(&GmtGridReader::SelectionModifiedCallback); // what's this?
  cbc->SetClientData(this);  // what's this?
}

GmtGridReader::~GmtGridReader() {
}




int GmtGridReader::RequestData(vtkInformation* request,
			       vtkInformationVector** inputVector,
			       vtkInformationVector* outputVector) {

  std::cerr << "GmtGridReader::RequestData()" << std::endl;
  
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // Check for valid output vtkDataSet
  vtkDataSet* output = vtkDataSet::GetData(outInfo);
  if (!output)
  {
    vtkErrorMacro(<< "Bad output type.");
    return 0;
  }  

  vtkPolyData* polyOutput = vtkPolyData::SafeDownCast(output);
  if (!polyOutput) {
    vtkErrorMacro(<< "Output is not vtkPolyData");
    return 0;
  }
  
  // Read grid file
  std::cerr << "GmtGridReader::RequestData() - readGridFile()" << std::endl;  
  void *gmtApi;
  gmtGrid_ = readGridFile(fileName_, &gmtApi);
  if (!gmtGrid_) {
    std::cerr << "error while reading " << fileName_ << std::endl;
    return 0;
  }

  // Reset/clear points
  gridPoints_->Reset();
  
  // Load points read from grid file
  std::cerr << "GmtGridReader::RequestData() - load points" << std::endl;    
  for (unsigned row = 0; row < gmtGrid_->header->n_rows; row++) {
    for (unsigned col = 0; col < gmtGrid_->header->n_columns; col++) {
      unsigned dataIndex = GMT_Get_Index(gmtApi, gmtGrid_->header, row, col);
      gridPoints_->InsertNextPoint(row, col,
				   gmtGrid_->data[dataIndex]);
    }
  }


  // Set triangle vertices
  unsigned nRows = gmtGrid_->header->n_rows;
  unsigned nCols = gmtGrid_->header->n_columns;
  vtkIdType triangleVertexId[3];
  // Triangles must stay within row and column bounds
  for (unsigned row = 0; row < nRows-1; row++) {
    for (unsigned col = 0; col < nCols-1; col++) {

      // First triangle
      triangleVertexId[0] = gridOffset(nRows, nCols, row, col);
      triangleVertexId[1] = gridOffset(nRows, nCols, row, col+1);
      triangleVertexId[2] = gridOffset(nRows, nCols, row+1, col+1);      
      gridPolygons_->InsertNextCell(3, triangleVertexId);
      
      // Second triangle
      triangleVertexId[0] = gridOffset(nRows, nCols, row, col);
      triangleVertexId[1] = gridOffset(nRows, nCols, row+1, col+1);
      triangleVertexId[2] = gridOffset(nRows, nCols, row+1, col);      
      gridPolygons_->InsertNextCell(3, triangleVertexId);
    }
  }


  polyOutput->SetPoints(gridPoints_);
  // std::cerr << "SKIPPING POLYGONS FOR NOW" << std::endl;
  polyOutput->SetPolys(gridPolygons_);  
  std::cerr << "GmtGridReader::RequestData() - done" << std::endl;
  return 1;
}


GMT_GRID *GmtGridReader::readGridFile(const char *gridFile, void **api) {
    fprintf(stderr, "readGridFile(): gridFile: %s\n", gridFile);
    // Check for file existence and readability
    struct stat fileStatus;

    if (stat(gridFile, &fileStatus) != 0
            || (fileStatus.st_mode & S_IFMT) == S_IFDIR
            || fileStatus.st_size <= 0) {
      std::cerr << "Can not read \"" << gridFile << "\"" << std::endl;
        return nullptr;
    }

    fprintf(stderr, "readGridFile(): create session\n");
    // Create GMT API
    *api =
            GMT_Create_Session("Topography::loadGrid()", 2U, 0U, nullptr);

    if (!*api) {
      std::cerr << "Could not get GMT API for \"" << gridFile << "\"" << std::endl;
        return nullptr;
    }

    fprintf(stderr, "gridFile now: %s\n", gridFile);

    GMT_GRID *grid = nullptr;
    // Try to read header and grid
    for (int nTry = 0; nTry < 100; nTry++) {
        grid = (struct GMT_GRID *)GMT_Read_Data(*api, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE,
                                                GMT_GRID_ALL, nullptr, gridFile, nullptr);
        if (grid) break;
        usleep(1000);
    }

    if (!grid) {
      std::cerr << "Unable to read GMT grid from \"" << gridFile << "\"" << std::endl;
        return nullptr;
    }
    return grid;
}


void GmtGridReader::SetFileName(const char *fileName) {
  if (fileName_) {
    free((void *)fileName_);
  }
  fileName_ = strdup((char *)fileName);
}


void GmtGridReader::SelectionModifiedCallback(vtkObject*, unsigned long, void* clientdata, void*)
{
  static_cast<GmtGridReader*>(clientdata)->Modified();
}


void GmtGridReader::zSpan(float *zMin, float *zMax) {
  *zMin = gmtGrid_->header->z_min;
  *zMax = gmtGrid_->header->z_max;
}


vtkIdType GmtGridReader::gridOffset(unsigned nRows, unsigned nCols, unsigned row, unsigned col) {
  if (row >= nRows || col >= nCols) { 
    // Out of bounds
    fprintf(stderr, "getGridOffset(): out-of-bounds: row=%d, nRows=%d, col=%d, nCols=%d\n",
	    row, nRows, col, nCols);
  }
    
  return (col + row * nCols);
}

