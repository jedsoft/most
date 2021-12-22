define slsh_main ()
{
   variable
     e1 = "\e[38;5;%d;48;5;%dm",
     e2 = "\e[38;2;%d;%d;%d;48;2;%d;%d;%dm",
     e3 = "\e[%d;%dm",
     i;

   variable rainbow = "";
   _for i (0, 76, 1)
     {
	variable r = 255-(255*i/76);
	variable g = (510*i/76);
	variable b = (255*i/76);
	if (g > 255) g = 510 - 255;
	rainbow = strcat (rainbow,
			  sprintf (e2, r, g, b, 255-r, 255-g, 255-b),
			  "-\e[0m");
     }
   rainbow = rainbow + "\n";
   () = fputs (rainbow, stdout);
   () = fputs ("\n", stdout);

   _for i (0, 7, 1)
     {
	() = fprintf (stdout, e1, 15, i);
	() = fprintf (stdout, "%4d", i);
     }
   _for i (8, 15, 1)
     {
	() = fprintf (stdout, e1, 0, i);
	() = fprintf (stdout, "%4d", i);
     }
   () = fputs ("\e[0m\n\n", stdout);
#iffalse
   _for i (40, 47, 1)
     {
	() = fprintf (stdout, e3, 37, i);
	() = fprintf (stdout, "%4d", i);
     }
   _for i (100, 107, 1)
     {
	() = fprintf (stdout, e3, 97, i);
	() = fprintf (stdout, "%4d", i);
     }
   () = fputs ("\e[0m\n\n", stdout);
#endif

   i = 16;
   loop (6)
     {
	loop (3)
	  {
	     loop (6)
	       {
		  () = fprintf (stdout, e1, 15, i);
		  () = fprintf (stdout, "%4d", i);
		  i++;
	       }
	  }
	i += 18;
	() = fputs ("\e[m\n", stdout);
     }
   () = fputs ("\n", stdout);

   i = 34;
   loop (6)
     {
	loop (3)
	  {
	     loop (6)
	       {
		  () = fprintf (stdout, e1, 0, i);
		  () = fprintf (stdout, "%4d", i);
		  i++;
	       }
	  }
	i += 18;
	() = fputs ("\e[m\n", stdout);
     }
   () = fputs ("\n", stdout);

   _for i (232, 243, 1)
     {
	() = fprintf (stdout, e1, 15, i);
	() = fprintf (stdout, "%4d", i);
     }
   () = fputs ("\e[m\n", stdout);
   _for i (244, 255, 1)
     {
	() = fprintf (stdout, e1, 0, i);
	() = fprintf (stdout, "%4d", i);
     }
   () = fputs ("\e[0m\n\n", stdout);

   () = fputs (rainbow, stdout);
}
