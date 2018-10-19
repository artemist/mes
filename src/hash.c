/* -*-comment-start: "//";comment-end:""-*-
 * GNU Mes --- Maxwell Equations of Software
 * Copyright © 2018 Jan (janneke) Nieuwenhuizen <janneke@gnu.org>
 *
 * This file is part of GNU Mes.
 *
 * GNU Mes is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * GNU Mes is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Mes.  If not, see <http://www.gnu.org/licenses/>.
 */

SCM make_vector__ (long k);
SCM vector_ref_ (SCM x, long i);
SCM vector_set_x_ (SCM x, long i, SCM e);

int
char_hash (int c)
{
  if (c >= 'a' && c <= 'z')
    return c - 'a';
  return 27;
}

int
hashq_ (SCM x, long size)
{
  int hash = char_hash (VALUE (CAR (STRING (x)))) * 27;
  if (TYPE (CDR (STRING (x))) == TPAIR)
    hash = hash + char_hash (VALUE (CADR (STRING (x))));
  else
    hash = hash + char_hash (0);
  assert (hash <= 756);
  return hash;
}

int
hashq (SCM x, SCM size)
{
  return hashq_ (x, VALUE (size));
}

SCM
hashq_ref (SCM table, SCM key, SCM dflt)
{
  unsigned hash = hashq_ (key, 0);
  SCM buckets = struct_ref_ (table, 4);
  SCM bucket = vector_ref_ (buckets, hash);
  SCM x = cell_f;
  if (TYPE (dflt) == TPAIR)
    x = CAR (dflt);
  if (TYPE (bucket) == TPAIR)
    x = assq (key, bucket);
  return x;
}

SCM
hashq_set_x (SCM table, SCM key, SCM value)
{
  unsigned hash = hashq_ (key, 0);
  SCM buckets = struct_ref_ (table, 4);
  SCM bucket = vector_ref_ (buckets, hash);
  if (TYPE (bucket) != TPAIR)
    bucket = cell_nil;
  bucket = acons (key, value, bucket);
  vector_set_x_ (buckets, hash, bucket);
  return value;
}

SCM
hash_table_printer (SCM table)
{
  fdputs ("#<", g_stdout); display_ (struct_ref_ (table, 2)); fdputc (' ', g_stdout);
  fdputs ("size: ", g_stdout); display_ (struct_ref_ (table, 3)); fdputc (' ', g_stdout);
  SCM buckets = struct_ref_ (table, 4);
  fdputs ("buckets: ", g_stdout);
  for (int i=0; i<LENGTH (buckets); i++)
    {
      SCM e = vector_ref_ (buckets, i);
      if (e != cell_unspecified)
        {
          fdputc ('[', g_stdout);
          while (TYPE (e) == TPAIR)
            {
              display_ (CAAR (e));
              e = CDR (e);
              if (TYPE (e) == TPAIR)
                fdputc (' ', g_stdout);
            }
          fdputs ("]\n  ", g_stdout);
        }
    }
  fdputc ('>', g_stdout);
}

SCM
make_hashq_type () ///((internal))
{
  SCM record_type_name = cstring_to_symbol ("<record-type>");
  SCM record_type = record_type_name; // FIXME
  SCM hashq_type_name = cstring_to_symbol ("<hashq-table>");
  SCM fields = cell_nil;
  fields = cons (cstring_to_symbol ("buckets"), fields);
  fields = cons (cstring_to_symbol ("size"), fields);
  fields = cons (fields, cell_nil);
  fields = cons (hashq_type_name, fields);
  return make_struct (record_type, fields, cell_unspecified);
}

SCM
make_hash_table_ (long size)
{
  if (!size)
    size = 30 * 27;
  SCM hashq_type_name = cstring_to_symbol ("<hashq-table>");
  SCM record_type_name = cstring_to_symbol ("<record-type>");
  //SCM hashq_type = hashq_type_name; // FIXME
  SCM hashq_type = make_hashq_type ();

  SCM buckets = make_vector__ (size);
  SCM values = cell_nil;
  values = cons (buckets, values);
  values = cons (MAKE_NUMBER (size), values);
  values = cons (hashq_type_name, values);
  return make_struct (hashq_type, values, cell_hash_table_printer);
}

SCM
make_hash_table (SCM x)
{
  long size = 0;
  if (TYPE (x) == TPAIR)
    {
      assert (TYPE (x) == TNUMBER);
      size = VALUE (x);
    }
  return make_hash_table_ (size);
}