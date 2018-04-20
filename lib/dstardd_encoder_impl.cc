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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "dstardd_encoder_impl.h"
#include "codec/dstardd.h"

namespace gr {
  namespace dstardd {

    dstardd_encoder::sptr
    dstardd_encoder::make(bool verbose)
    {
      return gnuradio::get_initial_sptr
        (new dstardd_encoder_impl(verbose));
    }

    /*
     * The private constructor
     */
    dstardd_encoder_impl::dstardd_encoder_impl(bool verbose)
      : gr::block("dstardd_encoder",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(0, 8*4096, sizeof(char))),
       d_verbose(verbose)
    {
      mkheader();
      dstar_init();
      message_port_register_in(pmt::mp("pdu"));
      set_msg_handler(pmt::mp("pdu"), boost::bind(&dstardd_encoder_impl::pdu, this, _1));
      set_output_multiple(8192);  // large enough for large packet
    }

    /*
     * Our virtual destructor.
     */
    dstardd_encoder_impl::~dstardd_encoder_impl()
    {
    }

    void
    dstardd_encoder_impl::mkheader() {
       d_header[0]=0xC0;
       d_header[1]=d_header[2]=0;
       memcpy(d_header+3, d_rptr1, 8);
       memcpy(d_header+11, d_rptr2, 8);
       memcpy(d_header+19, d_your, 8);
       memcpy(d_header+27, d_my1, 8);
       memcpy(d_header+35, d_my2, 4);
    }

    void
    dstardd_encoder_impl::pdu(pmt::pmt_t msg)
    {
       if(d_verbose) { std::cout << "Message Received\n"; }
       pmt::pmt_t vector = pmt::cdr(msg);
       size_t len = pmt::blob_length(vector);
       size_t offset = 0;
       unsigned char encoded[85+len+4];
       dstar_encode(d_header, reinterpret_cast<const unsigned char*>(pmt::u8vector_elements(vector).data()), len, encoded);
      
       boost::lock_guard<gr::thread::mutex> lock{d_mutex};
       if(d_encoded_ready) {  std::cout << "Encoded buffer overflow - dropping message\n"; }
       // 01-Sequence and Sync 
       memset(d_encoded, 0x55, d_prefixlen);
       memcpy(d_encoded+d_prefixlen, "\x76\x50", 2);
       memcpy(d_encoded+d_prefixlen+2, encoded, 85+len+4);
       d_encoded_ready = d_prefixlen + 2 + 85+len+4;
       d_cond.notify_one(); 
    }


    void
    dstardd_encoder_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int
    dstardd_encoder_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      unsigned char *out= (unsigned char *) output_items[0];
#if 0
      const <+ITYPE+> *in = (const <+ITYPE+> *) input_items[0];

      // Do <+signal processing+>
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
#endif
      std::cout << "working\n" << d_encoded_ready;
      boost::unique_lock<boost::mutex> lock(d_mutex);
      if(!d_encoded_ready) { d_cond.timed_wait(lock, boost::posix_time::milliseconds(1000)); }
      if(!d_encoded_ready) { return 0; }

      /* Process output data from dencoded */
      if(noutput_items < d_encoded_ready + 2 + d_prefixlen/8) {
         std::cout << "Not enough space for a block\n";
         return 0; // TODO FIXME -- should never happen
      }
      std::cout << "Output of a block\n" << d_encoded_ready;
      memcpy(out, d_encoded, d_encoded_ready);
      noutput_items = d_encoded_ready;
      d_encoded_ready=0;
      return noutput_items;
    }

  } /* namespace dstardd */
} /* namespace gr */

