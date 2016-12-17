/* -*-comment-start: "//";comment-end:""-*-
 * Mes --- Maxwell Equations of Software
 * Copyright © 2016 Jan Nieuwenhuizen <janneke@gnu.org>
 *
 * This file is part of Mes.
 *
 * Mes is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * Mes is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mes.  If not, see <http://www.gnu.org/licenses/>.
 */

SCM display_helper (FILE*, SCM , bool, char const*, bool);

SCM
display (SCM x) ///((arity . n))
{
  SCM e = car (x);
  SCM p = cdr (x);
  int fd = 1;
  if (TYPE (p) == PAIR && TYPE (car (p)) == NUMBER) fd = HITS (car (p));
  FILE *f = fd == 1 ? stdout : stderr;
  return display_helper (f, e, false, "", false);
}

SCM
newline (SCM p) ///((arity . n))
{
  int fd = 1;
  if (TYPE (p) == PAIR && TYPE (car (p)) == NUMBER) fd = VALUE (car (p));
  FILE *f = fd == 1 ? stdout : stderr;
  fputs ("\n", f);
  return cell_unspecified;
}

SCM
display_ (FILE* f, SCM x)
{
  return display_helper (f, x, false, "", false);
}

SCM
display_helper (FILE* f, SCM x, bool cont, char const *sep, bool quote)
{
  SCM r;
  fprintf (f, "%s", sep);
  switch (TYPE (x))
    {
    case CHAR:
      {
        char const *name = 0;
        if (VALUE (x) == char_nul.value) name = char_nul.name;
        else if (VALUE (x) == char_backspace.value) name = char_backspace.name;
        else if (VALUE (x) == char_tab.value) name = char_tab.name;
        else if (VALUE (x) == char_newline.value) name = char_newline.name;
        else if (VALUE (x) == char_vt.value) name = char_vt.name;
        else if (VALUE (x) == char_page.value) name = char_page.name;
        else if (VALUE (x) == char_return.value) name = char_return.name;
        else if (VALUE (x) == char_space.value) name = char_space.name;
        if (name) fprintf (f, "#\\%s", name);
        else fprintf (f, "#\\%c", VALUE (x));
        break;
      }
    case MACRO:
      fprintf (f, "(*macro* ");
      display_helper (f, g_cells[x].macro, cont, sep, quote);
      fprintf (f, ")");
      break;
    case NUMBER: fprintf (f, "%d", VALUE (x)); break;
    case PAIR:
      {
        if (car (x) == cell_circular) {
          fprintf (f, "(*circ* . #-1#)");
          return cell_unspecified;
        }
        if (car (x) == cell_closure) {
          fprintf (f, "(*closure* . #-1#)");
          return cell_unspecified;
        }
        if (car (x) == cell_symbol_quote) {
          fprintf (f, "'");
          x = cdr (x);
          if (TYPE (x) != FUNCTION)
            x = car (x);
          return display_helper (f, x, cont, "", true);
        }
        if (!cont) fprintf (f, "(");
        display_ (f, car (x));
        if (cdr (x) && TYPE (cdr (x)) == PAIR)
          display_helper (f, cdr (x), true, " ", false);
        else if (cdr (x) != cell_nil) {
          fprintf (f, " . ");
          display_ (f, cdr (x));
        }
        if (!cont) fprintf (f, ")");
        break;
      }
    case VECTOR:
      {
        fprintf (f, "#(");
        for (int i = 0; i < LENGTH (x); i++) {
          if (TYPE (VECTOR (x)+i) == VECTOR
              || (TYPE (VECTOR (x)+i) == REF
                  && TYPE (REF (VECTOR (x)+i)) == VECTOR))
            fprintf (f, "%s#(...)", i ? " " : "");
          else
            display_helper (f,VECTOR (x)+i, false, i ? " " : "", false);
        }
        fprintf (f, ")");
        break;
      }
    case REF: display_helper (f, g_cells[x].ref, cont, "", true); break;
    case FUNCTION:
      {
        fprintf (f, "#<procedure ");
        SCM p = STRING (x);
        if (p < 0 || p >= g_free.value || TYPE (p) != PAIR)
          fprintf (f, "%s", NAME (x));
        else
          display_ (f, STRING (x));
        fprintf (f, ">");
        break;
      }
    case BROKEN_HEART: fprintf (f, "<3"); break;
    case KEYWORD:
      fprintf (f, "#:");
    default:
      if (STRING (x))
        {
          SCM p = STRING (x);
          assert (p);
          while (p != cell_nil) {
            assert (TYPE (car (p)) == CHAR);
            fputc (VALUE (car (p)), f);
            p = cdr (p);
          }
        }
      else if (TYPE (x) != PAIR && NAME (x)) fprintf (f, "%s", NAME (x));
    }
  return cell_unspecified;
}