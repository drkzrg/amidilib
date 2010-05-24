/**  Copyright 2008, 2009 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.

    AMIDILIB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AMIDILIB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AMIDILIB.  If not, see <http://www.gnu.org/licenses/>.*/

#include "INCLUDE/IFF.H"

/*
ID GetChunkHeader();

we read the chunks in a FORM ILBM with a loop like this:


do
  switch (id = GetChunkHeader())
    {
    case 'CMAP': ProcessCMAP(); break;
    case 'BMHD': ProcessBMHD(); break;
    case 'BODY': ProcessBODY(); break;
 */
    /* default: just ignore the chunk */
  /*  }*/

  /*until (id == END_MARK);*/
