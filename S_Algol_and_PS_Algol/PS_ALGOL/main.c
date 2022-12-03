extern char * stop;
main()
{
		char c;
		stop = &c;
        layer3init();
		initctab();
#ifdef testing
printf("TESTING\n");
        test();
#endif
#ifndef  testing
        SAMAIN();
#endif
		layer3commit();
        commit();
        exit();
 	    return(0);
}