// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
    2 // vi:set ts=4 sts=4 sw=4 noet :
    3 // Copyright 2008-2012, The TPIE development team
    4 // 
    5 // This file is part of TPIE.
    6 // 
    7 // TPIE is free software: you can redistribute it and/or modify it under
    8 // the terms of the GNU Lesser General Public License as published by the
    9 // Free Software Foundation, either version 3 of the License, or (at your
   10 // option) any later version.
   11 // 
   12 // TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
   13 // WARRANTY; without even the implied warranty of MERCHANTABILITY or
   14 // FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
   15 // License for more details.
   16 // 
   17 // You should have received a copy of the GNU Lesser General Public License
   18 // along with TPIE.  If not, see <http://www.gnu.org/licenses/>
   19 
   20 #ifndef _AMI_SORT_H
   21 #define _AMI_SORT_H
   22 
   40 
   41 // Get definitions for working with Unix and Windows
   42 #include <tpie/portability.h>
   43 
   44 // The typename that actually does the sorting
   45 #include <tpie/sort_manager.h>
   46 #include <tpie/mergeheap.h>
   47 #include <tpie/internal_sort.h>
   48 
   49 #include <tpie/progress_indicator_base.h>
   50 #include <tpie/progress_indicator_null.h>
   51 #include <tpie/fractional_progress.h>
   52 
   53 #include <tpie/pipelining/merge_sorter.h>
   54 #include <tpie/file_stream.h>
   55 #include <tpie/uncompressed_stream.h>
   56 
   57 namespace tpie {
   58 
   59 namespace bits {
   60 
   65 template<typename Stream, typename T, typename Compare>
   66 void generic_sort(Stream & instream, Compare comp,
   67                   progress_indicator_base * indicator) {
   68 
   69     stream_size_type sz = instream.size();
   70 
   71     fractional_progress fp(indicator);
   72     fractional_subindicator push(fp, "sort", TPIE_FSI, sz, "Write sorted runs");
   73     fractional_subindicator merge(fp, "sort", TPIE_FSI, sz, "Perform merge heap");
   74     fractional_subindicator output(fp, "sort", TPIE_FSI, sz, "Write sorted output");
   75     fp.init(sz);
   76 
   77     instream.seek(0);
   78 
   79     merge_sorter<T, true, Compare> s(comp);
   80     s.set_available_memory(get_memory_manager().available());
   81     s.begin();
   82     push.init(sz);
   83     while (instream.can_read()) s.push(instream.read()), push.step();
   84     push.done();
   85     s.end();
   86 
   87     instream.truncate(0);
   88     s.calc(merge);
   89 
   90     output.init(sz);
   91     while (s.can_pull()) instream.write(s.pull()), output.step();
   92     output.done();
   93     fp.done();
   94     instream.seek(0);
   95 }
   96 
   97 template<typename Stream, typename T, typename Compare>
   98 void generic_sort(Stream & instream, Stream & outstream, Compare comp,
   99                   progress_indicator_base *indicator) {
  100 
  101     if (&instream == &outstream) {
  102         generic_sort<Stream, T, Compare>(instream, comp, indicator);
  103         return;
  104     }
  105 
  106     stream_size_type sz = instream.size();
  107 
  108     fractional_progress fp(indicator);
  109     fractional_subindicator push(fp, "sort", TPIE_FSI, sz, "Write sorted runs");
  110     fractional_subindicator merge(fp, "sort", TPIE_FSI, sz, "Perform merge heap");
  111     fractional_subindicator output(fp, "sort", TPIE_FSI, sz, "Write sorted output");
  112     fp.init(sz);
  113 
  114     instream.seek(0);
  115 
  116     merge_sorter<T, true, Compare> s(comp);
  117     s.set_available_memory(get_memory_manager().available());
  118     s.begin();
  119     push.init(sz);
  120     while (instream.can_read()) s.push(instream.read()), push.step();
  121     push.done();
  122     s.end();
  123 
  124     s.calc(merge);
  125 
  126     outstream.truncate(0);
  127     output.init(sz);
  128     while (s.can_pull()) outstream.write(s.pull()), output.step();
  129     output.done();
  130     fp.done();
  131     outstream.seek(0);
  132 }
  133 
  134 }
  135 
  140 template<typename T, typename Compare>
  141 void sort(uncompressed_stream<T> &instream, uncompressed_stream<T> &outstream,
  142           Compare comp, progress_indicator_base & indicator) {
  143     bits::generic_sort<uncompressed_stream<T>, T, Compare>(instream, outstream, &comp, &indicator);
  144 }
  145 
  149 template<typename T>
  150 void sort(uncompressed_stream<T> &instream, uncompressed_stream<T> &outstream,
  151           tpie::progress_indicator_base* indicator=NULL) {
  152     std::less<T> comp;
  153     sort(instream, outstream, comp, indicator);
  154 }
  155 
  159 template<typename T>
  160 void sort(file_stream<T> &instream, file_stream<T> &outstream,
  161           tpie::progress_indicator_base* indicator=NULL) {
  162     std::less<T> comp;
  163     bits::generic_sort<file_stream<T>, T>(instream, outstream, comp, indicator);
  164 }
  165 
  166 
  167 // ********************************************************************
  168 // *                                                                  *
  169 // * Duplicates of the above versions that only use 2x space and      *
  170 // * overwrite the original input stream                              *
  171 // *                                                                  *
  172 // ********************************************************************/
  173 
  178 template<typename T, typename Compare>
  179 void sort(uncompressed_stream<T> &instream, Compare comp,
  180           progress_indicator_base & indicator) {
  181     sort(instream, instream, comp, &indicator);
  182 }
  183 
  188 template<typename T, typename Compare>
  189 void sort(file_stream<T> &instream, Compare comp,
  190           progress_indicator_base & indicator) {
  191     bits::generic_sort<file_stream<T>, T>(instream, comp, &indicator);
  192 }
  193 
  197 template<typename T>
  198 void sort(uncompressed_stream<T> &instream, 
  199           progress_indicator_base &indicator) {
  200     sort(instream, instream, &indicator);
  201 }
  202 
  203 template<typename T>
  204 void sort(file_stream<T> &instream,
  205           progress_indicator_base &indicator) {
  206     std::less<T> comp;
  207     sort(instream, comp, indicator);
  208 }
  209 
  214 template <typename T>
  215 void sort(uncompressed_stream<T> & instream) {
  216     sort(instream, instream);
  217 }
  218 
  219 }  //  tpie namespace
  220 
  221 #include <tpie/sort_deprecated.h>
  222 
  223 #endif // _AMI_SORT_H 

