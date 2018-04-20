/* -*- c++ -*- */

#define DSTARDD_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "dstardd_swig_doc.i"

%{
#include "dstardd/dstardd_decoder.h"
#include "dstardd/dstardd_encoder.h"
%}


%include "dstardd/dstardd_decoder.h"
GR_SWIG_BLOCK_MAGIC2(dstardd, dstardd_decoder);
%include "dstardd/dstardd_encoder.h"
GR_SWIG_BLOCK_MAGIC2(dstardd, dstardd_encoder);
