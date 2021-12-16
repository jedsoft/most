define slsh_main ()
{
   variable
     e1 = "\e[38;5;%d;48;5;%dm",
     e2 = "\e[38;2;%d;%d;%dm",
     i;

   i = 0;
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
}
