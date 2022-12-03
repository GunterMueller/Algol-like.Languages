ackerman(m,n)int m,n;
{if (m==0 ) {return  n+1;}else if (n==0 ){return ackerman(m-1,1);} else
return ackerman( m-1,ackerman(m,n-1));
}
main()
{
int i;

for( i=1 ;i<= 10;i++){  ackerman(3,6);};
}


