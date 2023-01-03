require ("png");

define slsh_main ()
{
   variable file = __argv[1];;

   variable png = png_read (file);
   variable nrows = array_shape (png)[0];
   variable ncols = array_shape (png)[1];

   _for (0, nrows-1, 1)
     {
	variable i = ();
	_for (0, ncols-1, 1)
	  {
	     variable j = ();
	     variable rgb = png[i, j] & 0xFFFFFF;
	     () = fprintf (stdout, "\e[48;2;%d;%d;%dm ",
			   rgb>>16, (rgb>>8)&0xFF, (rgb&0xFF));
	  }
	() = fprintf (stdout, "\e[0m\n");
     }
}
