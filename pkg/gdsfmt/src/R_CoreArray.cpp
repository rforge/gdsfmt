// ===========================================================
//     _/_/_/   _/_/_/  _/_/_/_/    _/_/_/_/  _/_/_/   _/_/_/
//      _/    _/       _/             _/    _/    _/   _/   _/
//     _/    _/       _/_/_/_/       _/    _/    _/   _/_/_/
//    _/    _/       _/             _/    _/    _/   _/
// _/_/_/   _/_/_/  _/_/_/_/_/     _/     _/_/_/   _/_/
// ===========================================================
//
// R_CoreArray.cpp: Export the C routines of CoreArray allowing C++ exceptions
//
// Copyright (C) 2014	Xiuwen Zheng [zhengx@u.washington.edu]
//
// This file is part of CoreArray.
//
// CoreArray is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License Version 3 as
// published by the Free Software Foundation.
//
// CoreArray is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with CoreArray.
// If not, see <http://www.gnu.org/licenses/>.

/**
 *	\file     R_CoreArray.cpp
 *	\author   Xiuwen Zheng [zhengx@u.washington.edu]
 *	\version  1.0
 *	\date     2014
 *	\brief    Export the C routines of CoreArray allowing C++ exceptions
 *	\details
**/

#define COREARRAY_GDSFMT_PACKAGE

#include <R_GDS_CPP.h>
#include <string.h>
#include <R_ext/Rdynload.h>
#include <map>


namespace gdsfmt
{
	using namespace std;

	/// a list of GDS files in the gdsfmt package
	COREARRAY_DLL_LOCAL PdGDSFile GDSFMT_GDS_Files[GDSFMT_MAX_NUM_GDS_FILES];


	/// get the index in 'GDSFMT_GDS_Files' for NULL
	COREARRAY_DLL_LOCAL int GetEmptyFileIndex(bool throw_error=true)
	{
		for (int i=0; i < GDSFMT_MAX_NUM_GDS_FILES; i++)
		{
			if (GDSFMT_GDS_Files[i] == NULL)
				return i;
		}
		if (throw_error)
		{
			throw ErrGDSFmt(
				"You have opened %d GDS files, and no more is allowed!",
				GDSFMT_MAX_NUM_GDS_FILES);
		}
		return -1;
	}

	/// get the index in 'GDSFMT_GDS_Files' for file
	COREARRAY_DLL_LOCAL int GetFileIndex(PdGDSFile file, bool throw_error=true)
	{
		for (int i=0; i < GDSFMT_MAX_NUM_GDS_FILES; i++)
		{
			if (GDSFMT_GDS_Files[i] == file)
				return i;
		}
		if (throw_error)
		{
			throw ErrGDSFmt(
				"The GDS file has been closed, or invalid.");
		}
		return -1;
	}



	/// initialization and finalization
	class COREARRAY_DLL_LOCAL CInitObject
	{
	public:
		/// initialization
		CInitObject()
		{
			memset(GDSFMT_GDS_Files, 0, sizeof(GDSFMT_GDS_Files));
		}

		/// finalization
		~CInitObject()
		{
			for (int i=0; i < GDSFMT_MAX_NUM_GDS_FILES; i++)
			{
				if (GDSFMT_GDS_Files[i] != NULL)
				{
					try {
						delete GDSFMT_GDS_Files[i];
					}
					catch (...) { }
				}
			}
			memset(GDSFMT_GDS_Files, 0, sizeof(GDSFMT_GDS_Files));
		}
	};
	
	static CInitObject InitObject;
}

using namespace std;
using namespace CoreArray;
using namespace CoreArray::Parallel;
using namespace gdsfmt;


extern "C"
{

static const char *ERR_WRITE_ONLY =
	"Writable only and please call 'readmode.gdsn' before reading.";


// ===========================================================================
// R objects

/// convert "(CdGDSObj*)  -->  int"
COREARRAY_DLL_EXPORT PdGDSObj GDS_R_SEXP2Obj(SEXP Obj)
{
	if (TYPEOF(Obj) != INTSXP)
		throw ErrGDSFmt("Invalid GDS node object!");
	if (XLENGTH(Obj) != GDSFMT_NUM_INT_FOR_OBJECT)
		throw ErrGDSFmt("Invalid GDS node object!");

	PdGDSObj rv = NULL;
	memcpy(&rv, INTEGER(Obj), sizeof(rv));

	return rv;
}

/// convert "(CdGDSObj*)  -->  SEXP"
COREARRAY_DLL_EXPORT SEXP GDS_R_Obj2SEXP(PdGDSObj Obj)
{
	SEXP rv = NEW_INTEGER(GDSFMT_NUM_INT_FOR_OBJECT);
	memset(INTEGER(rv), 0, sizeof(int)*GDSFMT_NUM_INT_FOR_OBJECT);
	memcpy(INTEGER(rv), &Obj, sizeof(PdGDSObj));

	return rv;
}

/// detect whether a node is valid
COREARRAY_DLL_EXPORT void GDS_R_NodeValid(PdGDSObj Obj, C_BOOL ReadOrWrite)
{
	if (Obj == NULL)
		throw ErrGDSFmt("Internal error in 'GDS_R_NodeValid': Obj = NULL.");

	PdGDSObj vObj = Obj;
	PdGDSFolder Folder = vObj->Folder();
	while (Folder != NULL)
	{
		vObj = Folder;
		Folder = vObj->Folder();
	}
	// vObj is the root, and then get the GDS file
	CdGDSFile *file = vObj->GDSFile();

	if (file)
	{
		for (int i=0; i < GDSFMT_MAX_NUM_GDS_FILES; i++)
		{
			if (GDSFMT_GDS_Files[i] == file)
			{
			#ifdef COREARRAY_UNIX
				if (file->GetProcessID() != GetCurrentProcessID())
				{
					// in forked process
					if (ReadOrWrite)
					{
						// reading
						if (file->IfSupportForking())
						{
							file->SetProcessID();
						} else {
							throw ErrGDSFmt(
								"Not support forking, please open the GDS file with 'allow.fork=TRUE'.");
						}
					} else {
						throw ErrGDSFmt(
							"Creating/Opening and writing the GDS file should be in the same (forked) process.");
					}
				}
			#endif
				return;
			}
		}
	}

	throw ErrGDSFmt("The GDS file has been closed.");
}

/// detect whether a node is valid
COREARRAY_DLL_EXPORT void GDS_R_NodeValid_SEXP(SEXP Obj, C_BOOL ReadOrWrite)
{
	bool has_error = false;
	CORE_TRY
		GDS_R_NodeValid(GDS_R_SEXP2Obj(Obj), ReadOrWrite);
	CORE_CATCH(has_error = true);
	if (has_error) error(GDS_GetError());
}

/// return true, if Obj is a logical object in R
COREARRAY_DLL_EXPORT C_BOOL GDS_R_Is_Logical(PdGDSObj Obj)
{
	return Obj->Attribute().HasName(UTF7("R.logical"));
}

/// return true, if Obj is a factor variable
COREARRAY_DLL_EXPORT C_BOOL GDS_R_Is_Factor(PdGDSObj Obj)
{
	if (Obj->Attribute().HasName(UTF7("R.class")) &&
		Obj->Attribute().HasName(UTF7("R.levels")))
	{
		return (Obj->Attribute()[UTF7("R.class")].GetStr8() == "factor");
	} else
		return false;
}

/// return 1 used in UNPROTECT and set levels in 'val' if Obj is a factor in R;
/// otherwise return 0
COREARRAY_DLL_EXPORT int GDS_R_Set_IfFactor(PdGDSObj Obj, SEXP val)
{
	int nProtected = 0;

	if (Obj->Attribute().HasName(UTF7("R.class")) &&
		Obj->Attribute().HasName(UTF7("R.levels")))
	{
		if (Obj->Attribute()[UTF7("R.class")].GetStr8() == "factor")
		{
			if (Obj->Attribute()[UTF7("R.levels")].IsArray())
			{
				const TdsAny *p = Obj->Attribute()[UTF7("R.levels")].GetArray();
				C_UInt32 L = Obj->Attribute()[UTF7("R.levels")].GetArrayLength();

				SEXP levels;
				PROTECT(levels = NEW_CHARACTER(L));
				nProtected ++;
				for (C_UInt32 i=0; i < L; i++)
				{
					SET_STRING_ELT(levels, i, mkChar(p[i].
						GetStr8().c_str()));
				}

				SET_LEVELS(val, levels);
				SET_CLASS(val, mkString("factor"));

			} else if (Obj->Attribute()[UTF7("R.levels")].IsString())
			{
				SEXP levels;
				PROTECT(levels = NEW_CHARACTER(1));
				nProtected ++;
				SET_STRING_ELT(levels, 0, mkChar(Obj->Attribute()
					[UTF7("R.levels")].GetStr8().c_str()));

				SET_LEVELS(val, levels);
				SET_CLASS(val, mkString("factor"));
			}
		}
	}

	// output
	return nProtected;
}

/// return an R data object from a GDS object
COREARRAY_DLL_EXPORT SEXP GDS_R_Array_Read(PdSequenceX Obj, C_Int32 const* Start,
	C_Int32 const* Length, const C_BOOL *const Selection[])
{
	SEXP rv_ans = R_NilValue;
	int nProtected = 0;

	try
	{
		CdSequenceX::TSeqDimBuf St, Cnt;
		if (Start == NULL)
		{
			memset(St, 0, sizeof(St));
			Start = St;
		}
		if (Length == NULL)
		{
			Obj->GetDimLen(Cnt);
			Length = Cnt;
		}

		CdSequenceX::TSeqDimBuf ValidCnt;
		Obj->GetInfoSelection(Start, Length, Selection, NULL, NULL, ValidCnt);

		C_Int64 TotalCount;
		if (Obj->DimCnt() > 0)
		{
			TotalCount = 1;
			for (int i=0; i < Obj->DimCnt(); i++)
				TotalCount *= ValidCnt[i];
		} else {
			// defined in CoreArray, Obj->DimCnt() should be always > 0
			TotalCount = 0;
			throw ErrGDSFmt("Internal error in 'GDS_R_Array_Read'.");
		}

		if (TotalCount > 0)
		{
			#ifndef R_XLEN_T_MAX
			if (TotalCount > TdTraits<R_xlen_t>::Max())
				throw ErrGDSFmt("No support of long vectors, please use 64-bit R with version >=3.0!");
			#endif

			void *buffer;
			enum C_SVType SV;
			if (COREARRAY_SV_INTEGER(Obj->SVType()))
			{
				if (GDS_R_Is_Logical(Obj))
				{
					PROTECT(rv_ans = NEW_LOGICAL(TotalCount));
					buffer = LOGICAL(rv_ans);
				} else {
					PROTECT(rv_ans = NEW_INTEGER(TotalCount));
					nProtected += GDS_R_Set_IfFactor(Obj, rv_ans);
					buffer = INTEGER(rv_ans);
				}
				SV = svInt32;
			} else if (COREARRAY_SV_FLOAT(Obj->SVType()))
			{
				PROTECT(rv_ans = NEW_NUMERIC(TotalCount));
				buffer = REAL(rv_ans);
				SV = svFloat64;
			} else if (COREARRAY_SV_STRING(Obj->SVType()))
			{
				PROTECT(rv_ans = NEW_CHARACTER(TotalCount));
				buffer = NULL;
				SV = svStrUTF8;
			} else
				throw ErrGDSFmt("Invalid SVType of array-oriented object.");
			nProtected ++;

			// initialize dimension
			if (Obj->DimCnt() > 1)
			{
				SEXP dim;
				PROTECT(dim = NEW_INTEGER(Obj->DimCnt()));
				nProtected ++;
				int I = 0;
				for (int k=Obj->DimCnt()-1; k >= 0; k--)
					INTEGER(dim)[ I++ ] = ValidCnt[k];
				SET_DIM(rv_ans, dim);
			}

			if (buffer != NULL)
			{
				if (!Selection)
					Obj->rData(Start, Length, buffer, SV);
				else
					Obj->rDataEx(Start, Length, Selection, buffer, SV);
			} else {
				vector<string> strbuf(TotalCount);
				if (!Selection)
					Obj->rData(Start, Length, &strbuf[0], SV);
				else
					Obj->rDataEx(Start, Length, Selection, &strbuf[0], SV);
				for (size_t i=0; i < strbuf.size(); i++)
					SET_STRING_ELT(rv_ans, i, mkChar(strbuf[i].c_str()));
			}
		} else {
			if (COREARRAY_SV_INTEGER(Obj->SVType()))
			{
				if (GDS_R_Is_Logical(Obj))
				{
					PROTECT(rv_ans = NEW_LOGICAL(0));
				} else {
					PROTECT(rv_ans = NEW_INTEGER(0));
					nProtected += GDS_R_Set_IfFactor(Obj, rv_ans);
				}
			} else if (COREARRAY_SV_FLOAT(Obj->SVType()))
			{
				PROTECT(rv_ans = NEW_NUMERIC(0));
			} else if (COREARRAY_SV_STRING(Obj->SVType()))
			{
				PROTECT(rv_ans = NEW_CHARACTER(0));
			} else
				throw ErrGDSFmt("Invalid SVType of GDS object.");
			nProtected ++;
		}

		// unprotect the object
		if (nProtected > 0)
			UNPROTECT(nProtected);
	}
	catch (ErrAllocRead &E)
	{
		throw ErrGDSFmt(ERR_WRITE_ONLY);
	}

	return rv_ans;
}

/// apply user-defined function margin by margin
/** \param Num         [in] the number of GDS objects
 *  \param ObjList     [in] a list of GDS objects
 *  \param Margins     [in  margin indices starting from 0 with C orders
 *  \param Selection   [in] indicating selection
 *  \param Func        [in] a user-defined function
 *  \param Param       [in] the parameter passed to the user-defined function
**/
COREARRAY_DLL_EXPORT void GDS_R_Apply(int Num, PdSequenceX ObjList[],
	int Margins[], const C_BOOL *const * const Selection[],
	void (*InitFunc)(SEXP Argument, C_Int32 Count, PdArrayRead ReadObjList[],
		void *_Param),
	void (*LoopFunc)(SEXP Argument, C_Int32 Idx, void *_Param),
	void *Param, C_BOOL IncOrDec)
{
	if (Num <= 0)
		throw ErrGDSFmt("GDS_R_Apply: Invalid 'Num=%d'.", Num);
	if (!IncOrDec)
		throw ErrGDSFmt("GDS_R_Apply: The current implementation does not support 'IncOrDec=FALSE'.");

	// -----------------------------------------------------------
	// get dimension information

	vector<int> DimCnt(Num);
	vector< vector<C_Int32> > DLen(Num);
	vector<enum C_SVType> SVType(Num);
	// for -- loop
	for (int i=0; i < Num; i++)
	{
		SVType[i] = ObjList[i]->SVType();
		if (!(COREARRAY_SV_NUMERIC(SVType[i]) || COREARRAY_SV_STRING(SVType[i])))
			throw ErrGDSFmt("GDS_R_Apply: Not numeric or character-type data.");
		DimCnt[i] = ObjList[i]->DimCnt();
		DLen[i].resize(DimCnt[i]);
		ObjList[i]->GetDimLen(&(DLen[i][0]));
	}

	// -----------------------------------------------------------
	// margin should be integer

	for (int i=0; i < Num; i++)
	{
		if ((Margins[i] < 0) || (Margins[i] >= DimCnt[i]))
			throw ErrGDSFmt("GDS_R_Apply: Invalid 'Margins[%d]'!", i);
	}

	// -----------------------------------------------------------
	// initialize variables

	// array read object
	vector<CdArrayRead> Array(Num);
	vector<PdArrayRead> ArrayList(Num);
	for (int i=0; i < Num; i++)
	{
		ArrayList[i] = &Array[i];
		if (COREARRAY_SV_INTEGER(SVType[i]))
		{
			Array[i].Init(*ObjList[i], Margins[i], svInt32,
				Selection[i], false);
		} else if (COREARRAY_SV_FLOAT(SVType[i]))
		{
			Array[i].Init(*ObjList[i], Margins[i], svFloat64,
				Selection[i], false);
		} else if (COREARRAY_SV_STRING(SVType[i]))
		{
			Array[i].Init(*ObjList[i], Margins[i], svStrUTF8,
				Selection[i], false);
		} else
			throw ErrGDSFmt("GDS_R_Apply: Not support data type.");
	}

	// check the margin
	C_Int32 MCnt = Array[0].Count();
	for (int i=1; i < Num; i++)
	{
		if (Array[i].Count() != MCnt)
		{
			throw ErrGDSFmt(
				"GDS_R_Apply: ObjList[%d] should have the same number of "
				"elements as ObjList[0] marginally (Margins[%d] = Margins[0]).",
				i, i);
		}
	}

	// allocate internal buffer uniformly
	Balance_ArrayRead_Buffer(&(Array[0]), Array.size());

	// used in UNPROTECT
	int nProtected = 0;

	// initialize buffer pointers
	vector<void *> BufPtr(Num);
	SEXP Func_Argument;

	if (Num > 1)
	{
		PROTECT(Func_Argument = NEW_LIST(Num));
		nProtected ++;
	}

	for (int i=0; i < Num; i++)
	{
		SEXP tmp;
		if (COREARRAY_SV_INTEGER(SVType[i]))
		{
			if (GDS_R_Is_Logical(ObjList[i]))
			{
				PROTECT(tmp = NEW_LOGICAL(Array[i].MarginCount()));
			} else {
				PROTECT(tmp = NEW_INTEGER(Array[i].MarginCount()));
				nProtected += GDS_R_Set_IfFactor(ObjList[i], tmp);
			}
			nProtected ++;
			BufPtr[i] = INTEGER(tmp);
		} else if (COREARRAY_SV_FLOAT(SVType[i]))
		{
			PROTECT(tmp = NEW_NUMERIC(Array[i].MarginCount()));
			nProtected ++;
			BufPtr[i] = REAL(tmp);
		} else if (COREARRAY_SV_STRING(SVType[i]))
		{
			PROTECT(tmp = NEW_STRING(Array[i].MarginCount()));
			nProtected ++;
			BufPtr[i] = tmp;
		} else {
			tmp = R_NilValue;
			BufPtr[i] = NULL;
		}

		// init dim
		if (DimCnt[i] > 2)
		{
			SEXP dim;
			PROTECT(dim = NEW_INTEGER(DimCnt[i] - 1));
			nProtected ++;
			int I = 0;
			for (int k=DimCnt[i]-1; k >= 0; k--)
			{
				if (k != Margins[i])
					INTEGER(dim)[ I++ ] = Array[i].DimCntValid()[k];
			}

			if (tmp != R_NilValue) SET_DIM(tmp, dim);
		}

		if (Num > 1)
			SET_VECTOR_ELT(Func_Argument, i, tmp);
		else
			Func_Argument = tmp;
	}

	try
	{
		vector<string> buffer_string;
	
		// call the initial user-defined function
		(*InitFunc)(Func_Argument, Array[0].Count(), &ArrayList[0], Param);

		// for - loop
		while (!Array[0].Eof())
		{
			C_Int32 Idx = Array[0].MarginIndex();

			// read marginally
			for (int i=0; i < Num; i++)
			{
				if (!COREARRAY_SV_STRING(SVType[i]))
				{
					Array[i].Read(BufPtr[i]);
				} else {
					C_Int64 n = Array[i].MarginCount();
					if (n > buffer_string.size())
						buffer_string.resize(n);
					Array[i].Read(&buffer_string[0]);
					SEXP bufstr = (SEXP)BufPtr[i];
					for (C_Int64 i=0; i < n; i++)
					{
						SET_STRING_ELT(bufstr, i,
							mkChar(buffer_string[i].c_str()));
					}
				}
			}

			// call the user-defined function
			(*LoopFunc)(Func_Argument, Idx, Param);
		}

		if (nProtected > 0)
			UNPROTECT(nProtected);
	}
	catch (ErrAllocRead &E)
	{
		throw ErrGDSFmt(ERR_WRITE_ONLY);
	}
}


// ===========================================================================
// functions for file structure

COREARRAY_DLL_EXPORT PdGDSFile GDS_File_Create(const char *FileName)
{
	// to register CoreArray classes and objects
	RegisterClass();

	int gds_idx = GetEmptyFileIndex();
	PdGDSFile file = NULL;

	try {
		file = new CdGDSFile;
		file->SaveAsFile(FileName);
		GDSFMT_GDS_Files[gds_idx] = file;
	}
	catch (exception &E) {
		if (file) delete file;
		throw;
	}
	catch (const char *E) {
		if (file) delete file;
		throw;
	}
	catch (...) {
		if (file) delete file;
		throw;
	}
	return file;
}

COREARRAY_DLL_EXPORT PdGDSFile GDS_File_Open(const char *FileName,
	C_BOOL ReadOnly, C_BOOL ForkSupport)
{
	// to register CoreArray classes and objects
	RegisterClass();

	int gds_idx = GetEmptyFileIndex();
	PdGDSFile file = NULL;

	try {
		file = new CdGDSFile;
		if (!ForkSupport)
			file->LoadFile(FileName, ReadOnly);
		else
			file->LoadFileFork(FileName, ReadOnly);

		GDSFMT_GDS_Files[gds_idx] = file;
	}
	catch (exception &E) {
		string Msg = E.what();
		if ((file!=NULL) && !file->Log().List().empty())
		{
			Msg.append(sLineBreak);
			Msg.append("Log:");
			for (size_t i=0; i < file->Log().List().size(); i++)
			{
				Msg.append(sLineBreak);
				Msg.append(file->Log().List()[i].Msg);
			}
		}
		if (file) delete file;
		throw ErrGDSFmt(Msg);
	}
	catch (const char *E) {
		string Msg = E;
		if ((file!=NULL) && !file->Log().List().empty())
		{
			Msg.append(sLineBreak);
			Msg.append("Log:");
			for (size_t i=0; i < file->Log().List().size(); i++)
			{
				Msg.append(sLineBreak);
				Msg.append(file->Log().List()[i].Msg);
			}
		}
		if (file) delete file;
		throw ErrGDSFmt(Msg);
	}
	catch (...) {
		if (file) delete file;
		throw;
	}
	return file;
}

COREARRAY_DLL_EXPORT void GDS_File_Close(PdGDSFile File)
{
	int gds_idx = GetFileIndex(File, false);
	if (gds_idx >= 0)
		GDSFMT_GDS_Files[gds_idx] = NULL;
	if (File) delete File;
}

COREARRAY_DLL_EXPORT void GDS_File_Sync(PdGDSFile File)
{
	File->SyncFile();
}

COREARRAY_DLL_EXPORT PdGDSFolder GDS_File_Root(PdGDSFile File)
{
	return &File->Root();
}

COREARRAY_DLL_EXPORT PdGDSFile GDS_Node_File(PdGDSObj Node)
{
	return Node->GDSFile();
}

COREARRAY_DLL_EXPORT void GDS_Node_GetClassName(PdGDSObj Node, char *Out,
	size_t OutSize)
{
	string nm = Node->dName();
	if (Out)
		strncpy(Out, nm.c_str(), OutSize);
}

COREARRAY_DLL_EXPORT int GDS_Node_ChildCount(PdGDSFolder Node)
{
	return Node->NodeCount();
}

COREARRAY_DLL_EXPORT PdGDSObj GDS_Node_Path(PdGDSFolder Node,
	const char *Path, C_BOOL MustExist)
{
	if (MustExist)
		return Node->Path(T(Path));
	else
		return Node->PathEx(T(Path));
}


// ===========================================================================
// functions for attributes

COREARRAY_DLL_EXPORT int GDS_Attr_Count(PdGDSObj Node)
{
	return Node->Attribute().Count();
}

COREARRAY_DLL_EXPORT int GDS_Attr_Name2Index(PdGDSObj Node, const char *Name)
{
	return Node->Attribute().IndexName(T(Name));
}


// ===========================================================================
// functions for CdSequenceX

COREARRAY_DLL_EXPORT int GDS_Seq_DimCnt(PdSequenceX Obj)
{
	return Obj->DimCnt();
}

COREARRAY_DLL_EXPORT void GDS_Seq_GetDim(PdSequenceX Obj, C_Int32 OutBuffer[],
	size_t N_Buf)
{
	if (Obj->DimCnt() > (int)N_Buf)
		throw ErrCoreArray("Insufficient buffer in 'GDS_Seq_GetDim'.");
	Obj->GetDimLen(OutBuffer);
}

COREARRAY_DLL_EXPORT C_Int64 GDS_Seq_GetTotalCount(PdSequenceX Obj)
{
	return Obj->TotalCount();
}

COREARRAY_DLL_EXPORT enum C_SVType GDS_Seq_GetSVType(PdSequenceX Obj)
{
	return Obj->SVType();
}

COREARRAY_DLL_EXPORT unsigned GDS_Seq_GetBitOf(PdSequenceX Obj)
{
	return Obj->BitOf();
}

COREARRAY_DLL_EXPORT void GDS_Seq_rData(PdSequenceX Obj, C_Int32 const* Start,
	C_Int32 const* Length, void *OutBuf, enum C_SVType OutSV)
{
	Obj->rData(Start, Length, OutBuf, OutSV);
}

COREARRAY_DLL_EXPORT void GDS_Seq_rDataEx(PdSequenceX Obj, C_Int32 const* Start,
	C_Int32 const* Length, const C_BOOL *const Selection[], void *OutBuf,
	enum C_SVType OutSV)
{
	Obj->rDataEx(Start, Length, Selection, OutBuf, OutSV);
}

COREARRAY_DLL_EXPORT void GDS_Seq_wData(PdSequenceX Obj, C_Int32 const* Start,
	C_Int32 const* Length, const void *InBuf, enum C_SVType InSV)
{
	Obj->wData(Start, Length, InBuf, InSV);
}

COREARRAY_DLL_EXPORT void GDS_Seq_AppendData(PdSequenceX Obj, ssize_t Cnt,
	const void *InBuf, enum C_SVType InSV)
{
	Obj->Append(InBuf, Cnt, InSV);
}

COREARRAY_DLL_EXPORT void GDS_Seq_AppendString(PdSequenceX Obj,
	const char *Text)
{
	UTF8String val = Text;
	Obj->Append(&val, 1, svStrUTF8);
}


// ===========================================================================
// Functions for CdContainer - TdIterator

COREARRAY_DLL_EXPORT void GDS_Iter_GetStart(PdContainer Node, PdIterator Out)
{
	*Out = Node->atStart();
}

COREARRAY_DLL_EXPORT void GDS_Iter_GetEnd(PdContainer Node, PdIterator Out)
{
	*Out = Node->atEnd();
}

COREARRAY_DLL_EXPORT PdContainer GDS_Iter_GetHandle(PdIterator I)
{
	return I->Handler;
}

COREARRAY_DLL_EXPORT void GDS_Iter_Offset(PdIterator I, C_Int64 Offset)
{
	if (Offset == 1)
		++ (*I);
	else if (Offset == -1)
		-- (*I);
	else
		(*I) += Offset;
}

COREARRAY_DLL_EXPORT C_Int64 GDS_Iter_GetInt(PdIterator I)
{
	return I->toInt();
}

COREARRAY_DLL_EXPORT C_Float64 GDS_Iter_GetFloat(PdIterator I)
{
	return I->toFloat();
}

COREARRAY_DLL_EXPORT void GDS_Iter_GetStr(PdIterator I, char *Out, size_t Size)
{
	UTF8String s = UTF16toUTF8(I->toStr());
	if (Out)
		strncpy(Out, s.c_str(), Size);
}

COREARRAY_DLL_EXPORT void GDS_Iter_SetInt(PdIterator I, C_Int64 Val)
{
	I->IntTo(Val);
}

COREARRAY_DLL_EXPORT void GDS_Iter_SetFloat(PdIterator I, C_Float64 Val)
{
	I->FloatTo(Val);
}

COREARRAY_DLL_EXPORT void GDS_Iter_SetStr(PdIterator I, const char *Str)
{
	I->StrTo(PChartoUTF16(Str));
}

COREARRAY_DLL_EXPORT size_t GDS_Iter_RData(PdIterator I, void *OutBuf,
	size_t Cnt, enum C_SVType OutSV)
{
	return I->rData(OutBuf, Cnt, OutSV);
}

COREARRAY_DLL_EXPORT size_t GDS_Iter_WData(PdIterator I, const void *InBuf,
	size_t Cnt, enum C_SVType InSV)
{
	return I->wData(InBuf, Cnt, InSV);
}


// ===========================================================================
// functions for error

/// the last error message
COREARRAY_DLL_LOCAL string R_CoreArray_Error_Msg;

COREARRAY_DLL_EXPORT const char *GDS_GetError()
{
	return R_CoreArray_Error_Msg.c_str();
}

COREARRAY_DLL_EXPORT void GDS_SetError(const char *Msg)
{
	if (Msg)
	{
		if (Msg != GDS_GetError())
		{
			R_CoreArray_Error_Msg = Msg;
		}
	} else {
		R_CoreArray_Error_Msg.clear();
	}
}



// ===========================================================================
// functions for parallel computing

/// create a mutex object
COREARRAY_DLL_EXPORT PdThreadMutex GDS_Parallel_InitMutex()
{
	return new CdThreadMutex;
}

/// destroy the mutex object
COREARRAY_DLL_EXPORT void GDS_Parallel_DoneMutex(PdThreadMutex Obj)
{
	if (Obj) delete Obj;
}

/// lock the mutex object
COREARRAY_DLL_EXPORT void GDS_Parallel_LockMutex(PdThreadMutex Obj)
{
	if (Obj) Obj->Lock();
}

/// unlock the mutex object
COREARRAY_DLL_EXPORT void GDS_Parallel_UnlockMutex(PdThreadMutex Obj)
{
	if (Obj) Obj->Unlock();
}

/// initialize a thread suspending object
COREARRAY_DLL_EXPORT PdThreadsSuspending GDS_Parallel_InitSuspend()
{
	return new CdThreadsSuspending;
}

/// destroy the thread suspending object
COREARRAY_DLL_EXPORT void GDS_Parallel_DoneSuspend(PdThreadsSuspending Obj)
{
	if (Obj) delete Obj;
}

/// suspend the thread suspending object
COREARRAY_DLL_EXPORT void GDS_Parallel_Suspend(PdThreadsSuspending Obj)
{
	if (Obj) Obj->Suspend();
}

/// wake up the thread suspending object
COREARRAY_DLL_EXPORT void GDS_Parallel_WakeUp(PdThreadsSuspending Obj)
{
	if (Obj) Obj->WakeUp();
}


/// the object with multiple threads for parallel computing
static CParallelBase R_CoreArray_ParallelBase;

/// parallel computing
COREARRAY_DLL_EXPORT void GDS_Parallel_RunThreads(
	void (*Proc)(PdThread, int, void*), void *Param, int nThread)
{
	R_CoreArray_ParallelBase.SetNumThread(nThread);
	R_CoreArray_ParallelBase.RunThreads((CParallelBase::TProc)Proc, Param);
}


// ===========================================================================
/// functions for machine

// Return the number of available CPU cores in the system
COREARRAY_DLL_EXPORT int GDS_Mach_GetNumOfCPU()
{
	return Mach::GetNumberOfCPU();
}

/// Return the size in byte of level-1 cache memory
COREARRAY_DLL_EXPORT size_t GDS_Mach_GetL1CacheMemory()
{
	return Mach::GetL1CacheMemory();
}

/// Return the size in byte of level-2 cache memory
COREARRAY_DLL_EXPORT size_t GDS_Mach_GetL2CacheMemory()
{
	return Mach::GetL2CacheMemory();
}


// ===========================================================================
// functions for reading block by block

/// read an array-oriented object margin by margin
COREARRAY_DLL_EXPORT PdArrayRead GDS_ArrayRead_Init(PdSequenceX Obj,
	int Margin, enum C_SVType SVType, const C_BOOL *const Selection[],
	C_BOOL buf_if_need)
{
	PdArrayRead rv = new CdArrayRead;
	rv->Init(*Obj, Margin, SVType, Selection, buf_if_need);
	return rv;
}

/// free a 'CdArrayRead' object
COREARRAY_DLL_EXPORT void GDS_ArrayRead_Free(PdArrayRead Obj)
{
	if (Obj) delete Obj;
}

/// read data
COREARRAY_DLL_EXPORT void GDS_ArrayRead_Read(PdArrayRead Obj, void *Buffer)
{
	Obj->Read(Buffer);
}

/// return true, if it is of the end
COREARRAY_DLL_EXPORT C_BOOL GDS_ArrayRead_Eof(PdArrayRead Obj)
{
	return Obj->Eof();
}

/// reallocate the buffer with specified size with respect to array
COREARRAY_DLL_EXPORT void GDS_ArrayRead_BalanceBuffer(PdArrayRead array[],
	int n, C_Int64 buffer_size)
{
	Balance_ArrayRead_Buffer(array, n, buffer_size);
}



// ===========================================================================
// initialize the package 'gdsfmt'

void R_init_gdsfmt(DllInfo *info)
{
	static const char *pkg_name = "gdsfmt";
	#define REG(nm)    R_RegisterCCallable(pkg_name, #nm, (DL_FUNC)&nm)

	// R objects
	REG(GDS_R_SEXP2Obj);
	REG(GDS_R_Obj2SEXP);
	REG(GDS_R_NodeValid);
	REG(GDS_R_NodeValid_SEXP);
	REG(GDS_R_Is_Logical);
	REG(GDS_R_Is_Factor);
	REG(GDS_R_Set_IfFactor);
	REG(GDS_R_Array_Read);
	REG(GDS_R_Apply);

	// functions for file structure
	REG(GDS_File_Create);
	REG(GDS_File_Open);
	REG(GDS_File_Close);
	REG(GDS_File_Sync);
	REG(GDS_File_Root);

	REG(GDS_Node_File);
	REG(GDS_Node_GetClassName);
	REG(GDS_Node_ChildCount);
	REG(GDS_Node_Path);

	// functions for attributes
	REG(GDS_Attr_Count);
	REG(GDS_Attr_Name2Index);

	// functions for CdSequenceX
	REG(GDS_Seq_DimCnt);
	REG(GDS_Seq_GetDim);
	REG(GDS_Seq_GetTotalCount);
	REG(GDS_Seq_GetSVType);
	REG(GDS_Seq_GetBitOf);
	REG(GDS_Seq_rData);
	REG(GDS_Seq_rDataEx);
	REG(GDS_Seq_wData);
	REG(GDS_Seq_AppendData);
	REG(GDS_Seq_AppendString);

	// functions for TdIterator
	REG(GDS_Iter_GetStart);
	REG(GDS_Iter_GetEnd);
	REG(GDS_Iter_GetHandle);
	REG(GDS_Iter_Offset);
	REG(GDS_Iter_GetInt);
	REG(GDS_Iter_GetFloat);
	REG(GDS_Iter_GetStr);
	REG(GDS_Iter_SetInt);
	REG(GDS_Iter_SetFloat);
	REG(GDS_Iter_SetStr);
	REG(GDS_Iter_RData);
	REG(GDS_Iter_WData);

	// functions for error
	REG(GDS_GetError);
	REG(GDS_SetError);

	// functions for parallel computing
	REG(GDS_Parallel_InitMutex);
	REG(GDS_Parallel_DoneMutex);
	REG(GDS_Parallel_LockMutex);
	REG(GDS_Parallel_UnlockMutex);
	REG(GDS_Parallel_InitSuspend);
	REG(GDS_Parallel_DoneSuspend);
	REG(GDS_Parallel_Suspend);
	REG(GDS_Parallel_WakeUp);
	REG(GDS_Parallel_RunThreads);

	// functions for machine
	REG(GDS_Mach_GetNumOfCPU);
	REG(GDS_Mach_GetL1CacheMemory);
	REG(GDS_Mach_GetL2CacheMemory);

	// functions for reading block by block
	REG(GDS_ArrayRead_Init);
	REG(GDS_ArrayRead_Free);
	REG(GDS_ArrayRead_Read);
	REG(GDS_ArrayRead_Eof);
	REG(GDS_ArrayRead_BalanceBuffer);
}

} // extern "C"