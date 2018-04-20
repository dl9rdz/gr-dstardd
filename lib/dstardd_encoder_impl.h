/* -*- c++ -*- */
/* 
 * Copyright 2018 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_DSTARDD_DSTARDD_ENCODER_IMPL_H
#define INCLUDED_DSTARDD_DSTARDD_ENCODER_IMPL_H

#include <dstardd/dstardd_encoder.h>

namespace gr {
  namespace dstardd {

    class dstardd_encoder_impl : public dstardd_encoder
    {
     private:
      bool d_verbose;
      unsigned char d_header[41];
      char d_my1[9]="DL9RDZ  ";
      char d_my2[5]="SDR ";
      char d_your[9]="CQCQCQ  ";
      char d_rptr1[9]="DB0VOX A";
      char d_rptr2[9]="DB0VOX G";

      int d_prefixlen=16;  // bytes, i.e. 16 byte=128 bit
      char d_encoded[16+85+4+4000];
      int d_encoded_ready=0;

      boost::mutex d_mutex;
      boost::condition_variable_any d_cond;

      void pdu(pmt::pmt_t msg);
      void mkheader();


     public:
      dstardd_encoder_impl(bool verbose);
      ~dstardd_encoder_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace dstardd
} // namespace gr

#endif /* INCLUDED_DSTARDD_DSTARDD_ENCODER_IMPL_H */

