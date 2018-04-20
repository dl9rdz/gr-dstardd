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

#ifndef INCLUDED_DSTARDD_DSTARDD_DECODER_IMPL_H
#define INCLUDED_DSTARDD_DSTARDD_DECODER_IMPL_H

#include <dstardd/dstardd_decoder.h>

enum { DSTAR_SYNC, DSTAR_HEAD, DSTAR_DATA };
#define DSTAR_SYNC_MASK 0x00FFFFFFU
#define DSTAR_SYNC_FLAG 0x00557650U
#define DSTAR_HEADBITS (660+16)
#define DSTAR_MAXDATA 4096

namespace gr {
  namespace dstardd {

    class dstardd_decoder_impl : public dstardd_decoder
    {
     private:
      bool d_verbose;
      int d_state;
      int d_pattern;
      int d_bitcount;
      unsigned char d_headbits[DSTAR_HEADBITS];
      unsigned char d_data[DSTAR_MAXDATA];
      int d_datalen;

     public:
      dstardd_decoder_impl(bool verbose);
      ~dstardd_decoder_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace dstardd
} // namespace gr

#endif /* INCLUDED_DSTARDD_DSTARDD_DECODER_IMPL_H */

