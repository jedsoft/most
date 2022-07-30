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
			  "-");
     }
   rainbow += "\e[0m";

   () = fprintf (stdout, "\e[3mThe next line assumes a truecolor terminal:\e[0m\n");
   rainbow = rainbow + "\n";
   () = fputs (rainbow, stdout);

   () = fprintf (stdout, "\e[3mThe next line uses 16 colors:\n\e[0m");

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
   () = fputs ("\e[0m\n", stdout);

   () = fputs ("\e[3mThe following lines assume a 256-color terminal\n\e[m", stdout);
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
   () = fputs ("\e[0m\n", stdout);

   () = fprintf (stdout, "\e[3mThe next line assumes a truecolor terminal:\n\e[m");
   () = fputs ("\e[4m", stdout);
   () = fputs (rainbow, stdout);
}
