/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: kctape.cc,v 1.12 2002/02/12 17:24:14 torsten_paul Exp $
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <getopt.h>

#include <iostream.h>
#include <iomanip.h>

#include "libtape/kct.h"
#include "fileio/load.h"

bool
dump(istream *s, int addr)
{
  int c, x;
  bool end;

  c = s->get();
  if (c == EOF)
    {
      end = true;
      return false;
    }

  cout << "Block " << hex << setw(2) << setfill('0')
       << (c & 0xff) << "h (" << dec << (c & 0xff) << ")"
       << endl;

  end = false;
  for (int a = 0;a < 8;a++)
    {
      int c;
      char buf[16];

      cout << hex << setw(4) << setfill('0') << (addr + a * 16) << "h: ";
      x = 16;
      for (int b = 0;b < 16;b++)
        {
          c = s->get();
          if (c == EOF)
            {
              x = b;
              end = true;
              break;
            }
          buf[b] = c;
        }
      for (int b = 0;b < 16;b++)
        {
	  if (b == 8)
	    cout << "- ";
          if (b >= x)
            cout << "   ";
          else
	    cout << hex << setw(2) << setfill('0') << (buf[b] & 0xff) << " ";
        }
      cout << "| ";
      for (int b = 0;b < 16;b++)
        {
          if (b >= x)
            break;
          cout << (char)(isprint(buf[b]) ? buf[b] : '.');
        }
      cout << endl;
      if (end)
        break;
    }
  cout << endl;

  return !end;
}

static long
read_file(unsigned char *buf, FILE *f, int block, long *bytes)
{
  int a;
  long l, len;
  unsigned char tmp[129], *ptr, *old_ptr;


  ptr = buf;
  memset(tmp, 0, 129);
  tmp[0] = block++;
  l = fread(&tmp[1], 1, 128, f);
  *bytes = l;
  len = l + 1;
  memcpy(ptr, tmp, 129);
  ptr += 129;

  old_ptr = 0;
  while (242)
    {
      memset(tmp, 0, 129);
      l = fread(&tmp[1], 1, 128, f);
      *bytes += l;
      if (l == 0)
        if (old_ptr)
          {
            *old_ptr = 0xff;
            break;
          }
      len += l + 1;
      if (l < 128)
        block = 0xff;
      tmp[0] = block++;
      memcpy(ptr, tmp, 129);
      if (l < 128)
        break;

      old_ptr = ptr;
      ptr += 129;
    }

  return len;
}

static kct_error_t
add_file(KCTFile &kct_file, char *filename)
{
#if 0
  FILE *f;
  long len, bytes;
  unsigned char buf[255 * 129 + 128], *ptr;

  if (strlen(name) > KCT_NAME_LENGTH)
    {
      cerr << "ERROR: Filename too long" << endl;
      return KCT_ERROR_INVAL;
    }
  if ((f = fopen(filename, "rb")) == NULL)
    {
      cerr << "ERROR: Can't open `" << filename << "'" << endl;
      perror("ERROR");
      return KCT_ERROR_NOENT;
    }

  len = read_file(&buf[0], f, 1, &bytes);
  fclose(f);

  if (len > 255 * 129 + 128)
    {
      cerr << "ERROR: Invalid file `" << filename << "'" << endl;
      return KCT_ERROR_IO;
    }

  return kct_file.write(name, buf, len);
#endif

  kct_file_type_t type;
  fileio_prop_t *ptr, *prop;

  if (fileio_load_file(filename, &prop) != 0)
    return KCT_ERROR_IO;

  fileio_debug_dump(prop, 0);
  for (ptr = prop;ptr != NULL;ptr = ptr->next)
    {
      switch (prop->type)
	{
	case FILEIO_TYPE_COM:
	  type = KCT_TYPE_COM;
	  break;
	case FILEIO_TYPE_BAS:
	  type = KCT_TYPE_BAS;
	  break;
	case FILEIO_TYPE_PROT_BAS:
	  type = KCT_TYPE_BAS_P;
	  break;
	default:
	  cerr << "ERROR: file with unknown type ignored!" << endl;
	  continue;
	}
      kct_file.write((const char *)&ptr->name[0],
		     ptr->data, ptr->size,
		     ptr->load_addr, ptr->start_addr,
		     type, KCT_MACHINE_ALL);
    }

  return KCT_OK;
}

kct_error_t
add_raw_file(KCTFile &kct_file, char *filename, char *tapename,
             char *name, long load, long start)
{
  FILE *f;
  int end;
  long len, bytes;
  unsigned char buf[255 * 129 + 128];

  if (strlen(name) > KCT_NAME_LENGTH)
    {
      cerr << "ERROR: Filename too long" << endl;
      return KCT_ERROR_INVAL;
    }
  if ((f = fopen(filename, "rb")) == NULL)
    {
      cerr << "ERROR: Can't open `" << filename << "'" << endl;
      perror("ERROR");
      return KCT_ERROR_NOENT;
    }
  
  memset(buf, 0, 129);
  len = read_file(&buf[129], f, 2, &bytes);
  fclose(f);

  if (len > 32768)
    {
      cerr << "ERROR: Invalid file `" << filename << "'" << endl;
      return KCT_ERROR_IO;
    }

  buf[0] = 1;
  strcpy((char *)(buf + 1), name);
  if (start == 0xffff)
    buf[17] = 2;
  else
    buf[17] = 3;
  buf[18] = load & 0xff;
  buf[19] = (load >> 8) & 0xff;

  end += load + bytes;

  buf[20] = end & 0xff;
  buf[21] = (end >> 8) & 0xff;
  buf[22] = start & 0xff;
  buf[23] = (start >> 8) & 0xff;
  return kct_file.write(tapename, buf, len + 129, load, start, KCT_TYPE_COM);
}

static kct_error_t
_open(KCTFile &f, char *name)
{
  if (f.open(name) != KCT_OK)
    {
      cerr << "ERROR: Can't open file `" << name << "'" << endl;
      perror("ERROR");
      return KCT_ERROR_NOENT;
    }
  return KCT_OK;
}

int main(int argc, char **argv)
{
  int a, c, addr;
  istream *s;
  KCTFile kct_file;
  kct_file_props_t props;

  if ((argc < 2) ||
      (strcmp(argv[1], "-h") == 0) ||
      (strcmp(argv[1], "--help") == 0))
    {
      cout << "  _  ______ _\n"
	   << " | |/ / ___| |_ __ _ _ __   ___                KCtape 0.2\n"
	   << " | ' / |   | __/ _` | '_ \\ / _ \\       (c) 1997-2002 Torsten Paul\n"
	   << " | . \\ |___| || (_| | |_) |  __/         <Torsten.Paul@gmx.de>\n"
	   << " |_|\\_\\____|\\__\\__,_| .__/ \\___|      http://kcemu.sourceforge.net/\n"
	   << "                    |_|\n"
	   << "\n"
	   << "KCtape is part of KCemu the KC 85/4 Emulator and comes with\n"
	   << "ABSOLUTELY NO WARRANTY; for details run `kcemu --warranty'.\n"
	   << "This is free software, and you are welcome to redistribute it\n"
	   << "under certain conditions; run `kcemu --license' for details.\n"
	   << "\n"
	   << "usage: kctape tapefile [command [command_args]]\n"
	   << "\n"
           << " commands:\t\targuments:\n"
           << "  -l|--list\n"
           << "  -c|--create\n"
           << "  -a|--add\t\tfilename [filename] ...\n"
           << "  -1|--add1\t\tfilename [filename] ... (KC 85/1 mode)\n"
	   << "  -A|--add-raw\t\tfilename tapename kcname loadaddr [startaddr]\n"
	   << "  -x|--extract\t\ttapename\n"
           << "  -r|--remove\t\tname\n"
           << "  -d|--dump\n";

      //<< "  -b|--print-bam" << endl
      //<< "  -B|--print-block-list" << endl;

      return 0;
    }

  fileio_init();
  kct_file.test();

  /*
   *  LIST (default)
   */
  if ((argv[2] == NULL) ||
      (strcmp(argv[2], "-l") == 0) ||
      (strcmp(argv[2], "--list") == 0))
    {
      if (_open(kct_file, argv[1]) == KCT_OK)
        {
          kct_file.readdir();
          kct_file.list();
        }
    }
  /*
   *  CREATE
   */
  else if ((strcmp(argv[2], "-c") == 0) || (strcmp(argv[2], "--create") == 0))
    {
      if (kct_file.create(argv[1]) != KCT_OK)
        {
          cerr << "ERROR: Can't create file `" << argv[1] << "'" << endl;
          perror("ERROR");
        }
      kct_file.close();
    }
  /*
   *  ADD
   */
  else if ((strcmp(argv[2], "-a") == 0) ||
	   (strcmp(argv[2], "-1") == 0) ||
	   (strcmp(argv[2], "--add") == 0) ||
	   (strcmp(argv[2], "--add1") == 0))
    {
      int idx, ret;

      if ((strcmp(argv[2], "-1") == 0) || (strcmp(argv[2], "--add1") == 0))
	fileio_set_kctype(FILEIO_KC85_1);

      if (argc < 4)
        {
          cerr << "ERROR: Missing arguments for command `--add'" << endl
               << "Try `" << argv[0] << " --help' for more information."
               << endl;
          return 1;
        }

      ret = 0;
      idx = 3;
      if (_open(kct_file, argv[1]) == KCT_OK)
	{
          while (argv[idx] != NULL)
            {
              if (add_file(kct_file, argv[idx]) != KCT_OK)
                ret = 1;
	      idx++;
	    }
          return ret;
        }
    }
  /*
   *  ADD-RAW
   */
  else if ((strcmp(argv[2], "-A") == 0) || (strcmp(argv[2], "--add-raw") == 0))
    {
      char *endptr;
      long load, start;
      
      if (argc < 7)
        {
          cerr << "ERROR: Missing arguments for command `--add-raw'" << endl
               << "Try `" << argv[0] << " --help' for more information."
               << endl;
          return 1;
        }
      load = strtol(argv[6], &endptr, 0);
      if ((argv[6][0] == '\0') ||
          (*endptr != '\0') ||
          (load < 0) ||
          (load > 0xffff))
        {
          cerr << "ERROR: Specified loadaddress is invalid" << endl;
          return 1;
        }
      if (argc > 7)
        {
          start = strtol(argv[7], &endptr, 0);
          if ((argv[7][0] == '\0') ||
              (*endptr != '\0') ||
              (start < 0) ||
              (start > 0xffff))
            {
              cerr << "ERROR: Specified startaddress is invalid" << endl;
              return 1;
            }
        }
      else
        start = 0xffff;

      if (_open(kct_file, argv[1]) == KCT_OK)
        if (add_raw_file(kct_file, argv[3], argv[4], argv[5], load, start) != KCT_OK)
          return 1;
    }
  /*
   *  REMOVE
   */
  else if ((strcmp(argv[2], "-r") == 0) || (strcmp(argv[2], "--remove") == 0))
    {
      if (argc < 4)
        {
          cerr << "ERROR: Missing arguments for command `--remove'" << endl
               << "Try `" << argv[0] << " --help' for more information."
               << endl;
          return 1;
        }
      if (_open(kct_file, argv[1]) == KCT_OK)
        {
          kct_file.readdir();
          if (kct_file.remove(argv[3]) != KCT_OK)
            {
              cerr << "ERROR: File not found" << endl;
              return 1;
            }
        }
    }
  /*
   *  EXTRACT
   */
  else if ((strcmp(argv[2], "-x") == 0) ||
	   (strcmp(argv[2], "--extract") == 0))
    {
      if (argc < 4)
        {
          cerr << "ERROR: Missing arguments for command `--extract'" << endl
               << "Try `" << argv[0] << " --help' for more information."
               << endl;
          return 1;
        }
      if (_open(kct_file, argv[1]) == KCT_OK)
        {
	  kct_file.readdir();
	  s = kct_file.read(argv[3], &props);
	  if (s == NULL)
            {
              cerr << "ERROR: File not found" << endl;
              return 1;
            }

	  cerr << argv[3] << ": "
	       << "load = "  << hex << setw(4) << props.load_addr << ", "
	       << "size = "  << hex << setw(4) << props.size << ", "
	       << "start = " << hex << setw(4) << props.start_addr
	       << (props.auto_start ? " [autostart]" : "")
	       << endl;

	  fputs("\xc3KC-TAPE by AF. ", stdout);
	  a = 0;
	  while (242)
	    {
	      c = s->get();
	      if (c == EOF)
	        break;
	      a++;
	      fputc(c, stdout);
	    }
	  while ((a % 129) != 0)
            {
	      a++;
	      fputc('\0', stdout); /* pad to block size */
            }
        }
    }
  /*
   *  PRINT-BAM
   */
  else if ((strcmp(argv[2], "-b") == 0) ||
           (strcmp(argv[2], "--print-bam") == 0))
    {
      if (_open(kct_file, argv[1]) == KCT_OK)
        kct_file.print_bam();
    }
  /*
   *  PRINT-BLOCK-LIST
   */
  else if ((strcmp(argv[2], "-B") == 0) ||
           (strcmp(argv[2], "--print-block-list") == 0))
    {
      if (_open(kct_file, argv[1]) == KCT_OK)
        kct_file.print_block_list();
    }
  /*
   *  DUMP
   */
  else if ((strcmp(argv[2], "-d") == 0) ||
           (strcmp(argv[2], "--dump") == 0))
    {
      if (argc < 4)
        {
          cerr << "ERROR: Missing arguments for command `--dump'" << endl
               << "Try `" << argv[0] << " --help' for more information."
               << endl;
          return 1;
        }
      if (_open(kct_file, argv[1]) == KCT_OK)
        {
	  kct_file.readdir();
	  s = kct_file.read(argv[3], &props);
	  if (s == NULL)
            {
              cerr << "ERROR: File not found" << endl;
              return 1;
            }
          addr = 0;
          while (dump(s, addr))
            addr += 128;
        }
    }
  /*
   *  INVALID COMMAND!
   */
  else
    {
      cerr << "ERROR: Invalid option -- " << argv[2] << endl
           << "Try `" << argv[0] << " --help' for more information." << endl;
    }
  return 0;
}

