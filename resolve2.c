////////////////////////////////////////////////////////////////////////////
version="version resolve.lib 4.1.1.0 Dec_2017 "; // $Id: e1153f9d6e8f08095c748fb167696d7c8f135898 $
category="Algebraic Geometry";
info="
LIBRARY:  resolve.lib      Resolution of singularities (Desingularization)
                           Algorithm of Villamayor
AUTHORS:  A. Fruehbis-Krueger,  anne@mathematik.uni-kl.de,
@*        G. Pfister,           pfister@mathematik.uni-kl.de

REFERENCES:
[1] J.Kollar: Lectures on Resolution of Singularities, Princeton University Press (2007)@*
  (contains large overview over various known methods for curves and surfaces as well as@*
   a detailed description of the approach in the general case)@*
[2] A.Bravo, S.Encinas, O.Villamayor: A Simplified Proof of Desingularisation and@*
  Applications, Rev. Math. Iberoamericana 21 (2005), 349-458@*
  (description of the algorithmic proof of desingularization in characteristic zero
    which underlies this implementation)@*
[3] A.Fruehbis-Krueger: Computational Aspects of Singularities, in J.-P. Brasselet,
  J.Damon et al.: Singularities in Geometry and Topology, World Scientific
  Publishing, 253--327 (2007)@*
  (chapter 4 contains a detailed discussion on algorithmic desingularization and
  efficiency aspects thereof)

PROCEDURES:
 blowUp(J,C[,W,E]) computes the blowing up of the variety V(J) (considered
                   as embedded in V(W)) in the (smooth) center V(C),
 blowUp2(J,C)      computes the blowing up of the variety V(J) in the
                   (possibly singular) center V(C)
 Center(J[,W,E])   computes 'Villamayor'-center for blow up
 resolve(J)        computes the desingularization of the variety V(J)

 showBO(BO)      prints the content of a BO in more human readable form
 presentTree(L)  prints the final charts in more human readable form
 showDataTypes() prints help text for output data types

 blowUpBO(BO,C)  computes the blowing up of the variety V(BO[1]) in the
                 center V(C). BO is a list (basic object), C is an ideal
 createBO(J,W,E) creates basic object from input data
 CenterBO(BO)    computes the center for the next blow-up of the
                 given basic object
 Delta(BO)       apply the Delta-operator of [Bravo,Encinas,Villamayor]
 DeltaList(BO)   list of results of Delta^0 to Delta^bmax
";
LIB "elim.lib";
LIB "primdec.lib";
LIB "presolve.lib";
LIB "linalg.lib";
LIB "sing.lib";
///////////////////////////////////////////////////////////////////////////////
//                      Tasks:
//                     1) optimization of the local case
//                     2) optimization in Coeff
//                     3) change invariant to represent coeff=1 case
///////////////////////////////////////////////////////////////////////////////

proc showDataTypes()
"USAGE:  showDataTypes();
RETURN:  nothing, only pretty printing of extended version of help text
EXAMPLE: none
"
{
"                               ";
"//////////////// Short description of data type BO ///////////////////";
"BO[1] an ideal, say Wi, defining the ambient space of the i-th chart";
"      of the blowing up";
"BO[2] an ideal defining the strict transform";
"BO[3] intvec, the first integer b such that in the original object";
"      (Delta^b(BO[2]))==1";
"      the subsequent integers have the same property for Coeff-Objects";
"      of BO[2] used when determining the center";
"BO[4] the list of exceptional divisors";
"BO[5] an ideal defining the map K[W] ----> K[Wi]";
"BO[6] an intvec BO[6][j]=1 indicates that <BO[4][j],BO[2]>=1, i.e. the";
"      strict transform does not meet the j-th exceptional divisor";
"BO[7] intvec,";
"      the index of the first blown-up object in the resolution process";
"      leading to this object for which the value of b was BO[3]";
"      the subsequent ones are the indices for the Coeff-Objects";
"      of BO[2] used when determining the center";
"BO[i], i>7: internal data";
"                      ";
pause();
"              ";
"///////////// Short description of data in a chart ///////////////////";
"All chart data is stored in an object of type ring, the following ";
"variables are always present in such a ring:";
"BO:      list of type basic object containing the variety, ambient space,";
"         exceptional divisors and further data (see previous page)";
"cent:    ideal, describing the upcoming center determined by the algorithm";
"path:    module (autoconverted to matrix)";
"         path[1][i]= (i-1)st chart in resolution history of this chart";
"         path[2][i]= index of chart of the blow up leading to i-th chart";
"                     in resolution history";
"lastMap: ideal, describing the preceding blow up leading to this chart";
"                      ";
pause();
"                  ";
"///////////// Short description of type resolution data //////////////";
"list L containing two lists:";
"L[1]: list of rings, containing the final charts";
"L[2]: list of rings, containing all charts created in the resolution";
"      process";
"The most convenient way to view these data is the procedure presentTree()";
"from this library. Alternatively, it can be digested using tools from";
"the libraries reszeta.lib and resgraph.lib";
"//////////////////////////////////////////////////////////////////////";
}
///////////////////////////////////////////////////////////////////////////////
proc createBO(ideal J,list #)
"USAGE:  createBO(J[,W][,E]);
@*       J,W = ideals
@*       E     = list
ASSUME:  J  = ideal containing W ( W = 0 if not specified)
@*       E  = list of smooth hypersurfaces (e.g. exceptional divisors)
RETURN:  list BO representing a basic object :
         BO[1] ideal W, if W has been specified; ideal(0) otherwise
         BO[2] ideal J
         BO[3] intvec
         BO[4] the list E of exceptional divisors if specified;
               empty list otherwise
         BO[5] an ideal defining the identity map
         BO[6] an intvec
         BO[7] intvec
         BO[8] a matrix
         entries 3,5,6,7,8 are initialized appropriately for use of CenterBO
         and blowUpBO
EXAMPLE: example createBO;  shows an example
"
{
  ideal W;
  list E;
  ideal abb=maxideal(1);
  intvec v;
  intvec bvec;
  intvec w=-1;
  matrix intE;
  if(size(#)>0)
  {
    if(typeof(#[1])=="ideal")
    {
      W=#[1];
    }
    if(typeof(#[1])=="list")
    {
      E=#[1];
    }
    if(size(#)>1)
    {
      if((typeof(#[2])=="list") && (size(E)==0))
      {
        E=#[2];
      }
      if((typeof(#[2])=="ideal") && (size(W)==0))
      {
        W=#[2];
      }
    }
  }
  list BO=W,J,bvec,E,abb,v,w,intE;
  return(BO);
}
example
{"EXAMPLE:";
   echo = 2;
   ring R=0,(x,y,z),dp;
   ideal J=x2-y3;
   createBO(J,ideal(z));
}
///////////////////////////////////////////////////////////////////////////////

proc blowUp(ideal J,ideal C,list #)
"USAGE:  blowUp(J,C[,W][,E]);
         W,J,C = ideals,
         E     = list
ASSUME:  J  = ideal containing W ( W = 0 if not specified)
@*       C  = ideal containing J
@*       E  = list of smooth hypersurfaces (e.g. exceptional divisors)
NOTE:    W the ideal of the ambient space, C the ideal of the center of
         the blowup and J the ideal of the variety
         Important difference to blowUp2:
         - the ambient space V(W) is blown up and V(J) transformed in it
@*         - V(C) is assumed to be non-singular
COMPUTE: the blowing up of W in C, the exceptional locus, the strict
         transform of J and the blowup map
RETURN:  list, say l, of size at most size(C),

         l[i] is the affine ring corresponding to the i-th chart
         each l[i] contains the ideals
         - aS, ideal of the blownup ambient space
         - sT, ideal of the strict transform
         - eD, ideal of the exceptional divisor
         - bM, ideal corresponding to the blowup map

         l[i] also contains a list BO, which can best be viewed with showBO(BO)
         detailed information on the data type BO can be viewed via the
         command showDataTypes();
EXAMPLE: example blowUp;  shows an example
"
{
  def S=basering;
  ideal W;
  list E;
  ideal abb=maxideal(1);
  intvec v;
  intvec bvec;
  intvec w=-1;
  matrix intE;
  if(size(#)>0)
  {
    if(typeof(#[1])=="ideal")
    {
      W=#[1];
    }
    if(typeof(#[1])=="list")
    {
      E=#[1];
    }
    if(size(#)>1)
    {
      if((typeof(#[2])=="list") && (size(E)==0))
      {
        E=#[2];
      }
      if((typeof(#[2])=="ideal") && (size(W)==0))
      {
        W=#[2];
      }
    }
  }
  list BO=W,J,bvec,E,abb,v,w,intE;
  int locaT;
  export locaT;
  list blow=blowUpBO(BO,C,0);
  kill locaT;
  int i;
  for(i=1;i<=size(blow);i++)
  {
     def Q=blow[i];
     setring Q;
     ideal aS=BO[1];
     ideal sT=BO[2];
     ideal eD=BO[4][size(BO[4])];
     ideal bM=BO[5];
     kill lastMap;
     kill thisChart;
     export(aS);
     export(sT);
     export(eD);
     export(bM);
     blow[i]=Q;
     setring S;
     kill Q;
  }
  return(blow);
}
example
{"EXAMPLE:";
   echo = 2;
   ring R=0,(x,y),dp;
   ideal J=x2-y3;
   ideal C=x,y;
   list blow=blowUp(J,C);
   def Q=blow[1];
   setring Q;
   aS;
   sT;
   eD;
   bM;
}
///////////////////////////////////////////////////////////////////////////////

proc blowUp2(ideal J,ideal C)
"USAGE:  blowUp2(J,C);
         J,C = ideals,
ASSUME:  C  = ideal containing J
NOTE:    C the ideal of the center of the blowup and J the ideal
         of the variety
         Important differences to blowUp:
         - V(J) itself is blown up, not the ambient space
         - C is not assumed to be non-singular
COMPUTE: the blowing up of J in C, the exceptional locus and the blow-up
         map
RETURN:  list, say l, of size at most size(C),

         l[i] is the affine ring corresponding to the i-th chart
         each l[i] contains the ideals
         - Jnew, ideal of the blownup J
         - eD, ideal of the new exceptional divisor
         - bM, ideal corresponding to the blowup map

EXAMPLE: example blowUp2;  shows an example
"
{
//----------------------------------------------------------------------------
// Initialization and basic sanity checks of the input data
//----------------------------------------------------------------------------
  int i,j;
  def S=basering;
  def laM=maxideal(1);
//--- number of generators of C should be as small as possible
  def mstdC=mstd(C);
  if(ncols(mstdC[1])<ncols(mstdC[2]))
  {
     C=mstdC[1];
  }
  else
  {
     C=mstdC[2];
  }
  C=simplify(interred(C),2);
//--- the empty set is not a good center ;-)
  if(deg(lead(C[1]))==0)
  {
     ERROR("Your chosen center was the empty set. Exiting.");
  }
//--- V(C) should be a subset of V(J)
  if(size(reduce(J,std(C),5))>0)
  {
     ERROR("V(J) does not contain V(C). Exiting.");
  }
//---------------------------------------------------------------------------
// To compute the blowing up, we need to consider
//       (K[x_1,\dots,x_n]/J)[t*C[1],...,t*C[m]]
// which we want to represent as a quotient of K[x_1,...,x_n,y_1,..,y_m]
// by an ideal obtained by elimination of the extra variable t.
// (in the comments n=nvars(S) and m=ncols(C))
//---------------------------------------------------------------------------
//--- set up rings for the elimination
  string r1="ring R=("+charstr(basering)+"),(x(1.."+string(nvars(S));
  r1=r1+"),y(1.."+string(ncols(C))+")),dp;";
  string r1t="ring Rt=("+charstr(basering)+"),(x(1.."+string(nvars(S));
  r1t=r1t+"),y(1.."+string(ncols(C))+"),t),(dp,dp(1));";
  execute(r1);       // ring for describing the transforms
  execute(r1t);      // like r1, but with additional variable t
  def J=fetch(S,J);
  def C=fetch(S,C);
//--- we need to eliminate t from J,y(1)-t*C[1],...y(m)-t*C[m]
  ideal elId=J;
  for(i=1;i<=ncols(C);i++)
  {
     elId=elId,y(i)-t*C[i];
  }
  elId=eliminate(elId,t);   // ideal describing the transform of J
  setring R;                // get rid of t
  def elId=fetch(Rt,elId);
  def E=fetch(S,C);         // determine exceptional divisor
  E=E+elId;
  def laM0=fetch(S,laM);    // the blowup map
//-----------------------------------------------------------------------------
// The result is now represented in an A^n \times P^{m-1}, hence
// involving n+m variables. For further computations we would like
// to pass to charts to keep the total number of variables as low as
// possible
//----------------------------------------------------------------------------
  list resList;
  ideal Jsub;
  ideal Esub;
  ideal laM;
  ideal testId;
  map phi;
  list templist;
  for(i=1;i<=nvars(R)-nvars(S);i++)
  {
//--- first pass elId and E on to the i-th chart
      Jsub=std(subst(elId,y(i),1));
      if(deg(Jsub[1])==0)
      {
//--- transform does not meet this chart ==> ignore it
         i++;
         continue;
      }
      Esub=std(subst(E,y(i),1));
//--- now get rid of unnecessary variables
//--- first by appropriate coordinate changes
      templist=elimpart(Jsub);
      if(size(templist[2])>0)
      {
         phi=R,templist[5];
         Jsub=phi(Jsub);
         Jsub=simplify(interred(Jsub),2);
         Esub=phi(Esub);
         Esub=simplify(interred(Esub),2);
         laM=phi(laM0);
      }
      else
      {
         laM=laM0;
      }
//--- then by dropping non-occuring variables
      testId=Jsub,Esub,laM;
      templist=findvars(testId);
      if(size(templist[1])<nvars(R))
      {
         for(j=1;j<=size(templist[4]);j++)
         {
            laM=subst(laM,var(templist[4][j]),0);
         }
         execute("ring Rnew=("+charstr(basering)+"),("+string(templist[1])+"),dp;");
         ideal Jnew=imap(R,Jsub);
         ideal eD=imap(R,Esub);
         ideal bM=imap(R,laM);
      }
      else
      {
         def Rnew=basering;
         ideal Jnew=Jsub;
         ideal eD=Esub;
         ideal bM=laM;
      }
//--- export the relevant data of this ring and add the ring to the list
      export Jnew;
      export eD;
      export bM;
      resList[size(resList)+1]=Rnew;
      setring R;
      kill Rnew;
  }
  setring S;
  return(resList);
}
example
{  "EXAMPLE:";
   echo = 2;
   ring r=0,(x,y,z),dp;
   ideal I=z2-x^3*y^2;
   ideal C=z,xy;
   list li=blowUp2(I,C);
   size(li);                  // number of charts
   def S1=li[1];
   setring S1;                // chart 1
   basering;
   Jnew;
   eD;
   bM;
   def S2=li[2];
   setring S2;                // chart 2
   basering;
   Jnew;
   eD;
   bM;
}
///////////////////////////////////////////////////////////////////////////////

proc Center(ideal J,list #)
"USAGE:  Center(J[,W][,E])
@*       J,W = ideals
@*       E   = list
ASSUME:  J  = ideal containing W ( W = 0 if not specified)
@*       E  = list of smooth hypersurfaces (e.g. exceptional divisors)
COMPUTE: the center of the blow-up of J for the resolution algorithm
         of [Bravo,Encinas,Villamayor]
RETURN:  ideal, describing the center
EXAMPLE: example Center;  shows an example
"
{
  ideal W;
  list E;
  ideal abb=maxideal(1);
  intvec v;
  intvec bvec;
  intvec w=-1;
  matrix intE;
  if(size(#)>0)
  {
    if(typeof(#[1])=="ideal")
    {
      W=#[1];
    }
    if(typeof(#[1])=="list")
    {
      E=#[1];
    }
    if(size(#)>1)
    {
      if((typeof(#[2])=="list") && (size(E)==0))
      {
        E=#[2];
      }
      if((typeof(#[2])=="ideal") && (size(W)==0))
      {
        W=#[2];
      }
      if(size(#)==3){bvec=#[3];}
    }
  }

  list BO=W,J,bvec,E,abb,v,w,intE,intvec(0);
  if(defined(invSat)){kill invSat;}
  list invSat=ideal(0),intvec(0);
  export(invSat);

  list re=CenterBO(BO);
  ideal cent=re[1];
  return(cent);
}
example
{  "EXAMPLE:";
   echo = 2;
   ring R=0,(x,y),dp;
   ideal J=x2-y3;
   Center(J);
}
///////////////////////////////////////////////////////////////////////////////

proc blowUpBO(list BO, ideal C,int e)
"USAGE:  blowUpBO (BO,C,e);
@*       BO = basic object, a list: ideal W,
@*                                  ideal J,
@*                                  intvec b,
@*                                  list Ex,
@*                                  ideal ab,
@*                                  intvec v,
@*                                  intvec w,
@*                                  matrix M
@*       C  = ideal
@*       e  = integer (0 usual blowing up, 1 deleting extra charts, 2 deleting
@*            no charts )
ASSUME:  R  = basering, a polynomial ring, W an ideal of R,
@*       J  = ideal containing W,
@*       C  = ideal containing J
COMPUTE: the blowing up of BO[1] in C, the exeptional locus, the strict
         transform of BO[2]
NOTE:    blowUpBO may be applied to basic objects in the sense of
@*       [Bravo, Encinas, Villamayor] in the following referred to as BO
RETURN:  a list l of length at most size(C),
         l[i] is a ring containing an object BO:
@*         BO[1] an ideal, say Wi, defining the ambient space of the
               i-th chart of the blowing up
@*         BO[2] an ideal defining the strict transform
@*         BO[3] intvec, the first integer b such that in the original object
               (Delta^b(BO[2]))==1
               the subsequent integers have the same property for Coeff-Objects
               of BO[2] used when determining the center
@*         BO[4] the list of exceptional divisors
@*         BO[5] an ideal defining the map K[W] ----> K[Wi]
@*         BO[6] an intvec BO[6][j]=1 indicates that <BO[4][j],BO[2]>=1,
               i.e. the strict transform does not meet the j-th exceptional
               divisor
@*         BO[7] intvec,
               the index of the first blown-up object in the resolution process
               leading to this object for which the value of b was BO[3]
               the subsequent ones are the indices for the Coeff-Objects
               of BO[2] used when determining the center
@*         BO[8] a matrix indicating that BO[4][i] meets BO[4][j] by
               BO[8][i,j]=1 for i < j
@*         BO[9] empty

EXAMPLE: example blowUpBO;  shows an example
"
{
//---------------------------------------------------------------------------
// Initialization  and sanity checks
//---------------------------------------------------------------------------
   def R0=basering;
   if(!defined(locaT)){int locaT;}
   if(locaT){poly pp=@p;}
   intvec v;
   int shortC=defined(shortcut);
   int invS=defined(invSat);
   int eq,hy;
   int extra,noDel,keepDiv;
   if(e==1){extra=1;}
//---keeps all charts
   if(e==2){noDel=1;}
//---this is only for curves and surfaces
//---keeps all charts with relevant informations on the exceptional divisors
   if(e==3){keepDiv=1;}
   if( typeof(attrib(BO[2],"isEqui"))=="int" )
   {
      eq=attrib(BO[2],"isEqui");
   }
   if( typeof(attrib(BO[2],"isHy"))=="int" )
   {
      hy=attrib(BO[2],"isHy");
   }
   string newvar;
   int n=nvars(R0);
   int i,j,l,m,x,jj,ll,haveCenters,co;
//---the center should neither be the whole space nor empty
   if((size(C)==0)||(deg(C[1])==0))
   {
      list result=R0;
      return(result);
   }
   if(!defined(debugBlowUp))
   {
     int debugBlowUp=0;
   }
//---------------------------------------------------------------------------
// Drop unnecessary variables
//---------------------------------------------------------------------------
//---step 1: substitution
   if(!((keepDiv)||(noDel)))
   {
//!!! in case keepDiv and noDel:
//!!! maybe simplify the situation by an appropriate coordinate change
//!!! of this kind -- without dropping variables?
      list L=elimpart(BO[1]);
      if(size(L[2])!=0)
      {
         map psi=R0,L[5];
         C=psi(C);
         BO=psi(BO);
      }

      if(size(BO[1])==0)
      {
         ideal LL;
         for(j=1;j<=size(BO[4]);j++)
         {
            LL=LL,BO[4][j];
         }
         LL=variables(LL);
         L=elimpart(BO[2]);
         if((size(L[2])!=0)&&(size(std(LL+L[2]))==size(L[2])+size(LL)))
         {
            map chi=R0,L[5];
            C=chi(C);
            BO=chi(BO);
         }
      }
//---step 2: dropping non-occurring variables
      int s=size(C);
      ideal K=BO[1],BO[2],C;
      for(j=1;j<=size(BO[4]);j++)
      {
        K=K,BO[4][j];
      }
      list N=findvars(K);
      if(size(N[1])<n)
      {
         newvar=string(N[1]);
         v=N[4];
         for(j=1;j<=size(v);j++){BO[5]=subst(BO[5],var(v[j]),0);}
         execute("ring R1=("+charstr(R0)+"),("+newvar+"),dp;");
         list BO=imap(R0,BO);
         ideal C=imap(R0,C);
         n=nvars(R1);
      }
      else
      {
         def R1=basering;
      }
   }
   else
   {
     int s=size(C);
     def R1=basering;
   }
   if(debugBlowUp)
   {
     "---> In BlowUp: After dropping unnecessary variables";
     "BO:";
     BO;
     "C:";
     C;
   }
//---------------------------------------------------------------------------
// Do the actual blow-up
//---------------------------------------------------------------------------
//--- control the names of the variables
   execute("ring R=("+charstr(R0)+"),(x(1..n)),dp;");
   list BO=fetch(R1,BO);
   ideal C=fetch(R1,C);
   list Cmstd=mstd(C);
   C=Cmstd[2];
   if(size(Cmstd[1])<=size(Cmstd[2]))
   {
      C=Cmstd[1];
   }
   else
   {
      C=interred(C);
   }
   list result;
//--- the blow-up process
   ideal W  =BO[1];
   ideal J  =BO[2];
   intvec bvec  =BO[3];
   list Ex  =BO[4];
   ideal abb=BO[5];
   intvec wvec=BO[7];
   ideal laM=maxideal(1);
   if((typeof(BO[9])=="intmat")||(typeof(BO[9])=="intvec"))
   {
      def @invmat=BO[9];
   }
   if(size(BO)>9)
   {
//--- check whether a previous center had been split into connected components
      if(size(BO[10])>0)
      {
         list knownCenters=BO[10];
         haveCenters=1;
       }
   }

   matrix intE=BO[8];
   Ex[size(Ex)+1]=var(1);
         //to have the list depending on R in case BO[4] is empty

   execute("ring S=("+charstr(R)+"),("+varstr(R)+",y(0..s-1)),dp;");
   list resu;
   list B;
   execute("ring T=("+charstr(R)+"),("+varstr(R)+",y(0..s-1),t),dp;");
   ideal C=imap(R,C);
   ideal W=imap(R,W);
   execute("map phi=S,"+varstr(R)+",t*C;")
   setring S;

//--- the ideal describing the blow-up map
   ideal abb=imap(R,abb);
   ideal laM0=imap(R,laM);
//--- the ideal of the blowing up of the ambient space
   ideal W=preimage(T,phi,W);
//--- the ideal of the exceptional locus
   ideal E=imap(R,C);

   list E1=imap(R,Ex);
   E1[size(E1)]=E;
   ideal J=imap(R,J)+W;

   if(haveCenters){list kN=imap(R,knownCenters);}

//---  the strict transform of the exceptional divisors
   for(j=1;j<size(E1);j++){E1[j]=sat(E1[j]+W,E)[1];}
//---  the intersection matrix of the exceptional divisors
   matrix intEold=imap(R,intE);
   matrix intE[size(E1)][size(E1)];
   ideal U,Jsub,sLstd;

   for(j=1;j<size(E1);j++)
   {
       for(l=j+1;l<=size(E1);l++)
       {
          if(deg(E1[j][1])==0)
          {
             if(l<size(E1)){intE[j,l]=intEold[j,l];}
             else             {intE[j,l]=0;}
          }
          else
          {
             if(deg(E1[l][1])==0)
             {
                if(l<size(E1)){intE[j,l]=intEold[j,l];}
             }
             else
             {
                U= std(E1[l]+E1[j]);
                if(dim(U)>0){intE[j,l]=1;}
                else        {intE[j,l]=0;}
             }
          }
       }
   }
   if(debugBlowUp)
   {
     "----> In BlowUp: After Blowing-up, before Clean-Up";
     "W:";
     W;
     "J:";
     J;
   }
//----------------------------------------------------------------------------
// generating and cleaning up the different charts
//----------------------------------------------------------------------------
   list M;
   map psi;
   list E2;
   ideal K,JJ,laM,LL,MM;
   n=nvars(S);
   list N;
   list Bstd;
   intvec delCharts,extraCharts;
   delCharts[s]=0;
   extraCharts[s]=0;
   ideal MA=y(0..s-1);
   list ZRes,ZlaM,ZsLstd;
   for(i=0;i<=s-1;i++)
   {
     if(haveCenters)
     {
        B[10]=kN;
        for(j=1;j<=size(kN);j++)
        {
           B[10][j][1]=subst(B[10][j][1],y(i),1);
        }
     }
     B[8]=intE;
     B[1]=std(subst(W,y(i),1));
     if(deg(B[1][1])==0)
     {
//--- subsets of the empty set are not really interesting!
       delCharts[i+1]=1;
       ZRes[i+1]=B;
       ZlaM[i+1]=laM;
       i++;
       continue;
     }
     Jsub=subst(J,y(i),1);
     attrib(Jsub,"isEqui",eq);
     attrib(Jsub,"isHy",hy);
     B[2]=Jsub;
     B[3]=bvec;
     for(j=1;j<size(E1);j++){E2[j]=subst(E1[j],y(i),1);}
     E2[size(E1)]=E+B[1];
     B[4]=E2;
     M=elimpart(B[1]);
     B[5]=abb;
     laM=laM0;
     psi=S,maxideal(1);
     if(size(M[2])!=0)
     {
        psi=S,M[5];
        B=psi(B);
        laM=psi(laM);
     }
     Jsub=B[2];
     B[2]=sat(Jsub,std(psi(E)))[1];
     if(!defined(MAtmp)){ideal MAtmp=MA;}
     MAtmp[i+1]=0;
     JJ=std(B[2]+MAtmp);
     if(deg(JJ[1])==0)
     {
        delCharts[i+1]=1;
//--- the i-th chart will be marked for deleting because all informations
//--- are already contained in the union of the remaining charts
     }
     else
     {
        if((eq)&&(dim(JJ)<dim(std(B[2]))))
        {
           extraCharts[i+1]=1;
//---  compute the singular locus
           if((B[3][1]<=1)&&(hy))
//--- B[2] is a smooth hypersurface
           {
              sLstd=ideal(1);
           }
           else
           {
              Bstd=mstd(B[2]);
              if(n-dim(Bstd[1])>4)
              {
//--- in this case the singular locus is too complicated
                 sLstd=ideal(0);
              }
              JJ=Bstd[2];
              attrib(JJ,"isEqui",eq);
              B[2]=JJ;
              sLstd=slocusE(B[2]);
           }
           m=0;
           if(deg(std(sLstd+MAtmp)[1])==0)
           {
//--- the singular locus of B[2] is in the union of the remaining charts
              m=1;
              for(l=1;l<=size(B[4]);l++)
              {
                 if(deg(std(B[2]+B[4][l]+MAtmp)[1])!=0)
                 {
//--- the exceptional divisor meets B[2] at the locus of MAtmp
//--- we continue only if the option extra=1 and we have transversal
//--- intersection
                    m=0;
                    break;
                  }
              }
           }
           if(m)
           {
//--- the i-th chart will be marked for deleting because all informations
//--- are already contained in the union of the remaining charts
              delCharts[i+1]=1;
           }
        }
     }
     if(delCharts[i+1]==0)
     {
        MAtmp[i+1]=MA[i+1];
        ZsLstd[i+1]=sLstd;
     }
     ZRes[i+1]=B;
     ZlaM[i+1]=laM;
   }
//---------------------------------------------------------------------------
// extra = ignore uninteresting charts even if there is a normal
//         crosssing intersection in it
//---------------------------------------------------------------------------
   if(extra)
   {
      for(i=0;i<=s-1;i++)
      {
         if((delCharts[i+1]==0)&&(extraCharts[i+1]))
         {
           MAtmp[i+1]=0;
           B=ZRes[i+1];
           sLstd=ZsLstd[i+1];
           m=0;
           if(deg(std(sLstd+MAtmp)[1])==0)
           {
//--- the singular locus of B[2] is in the union of the remaining charts
              m=1;
              for(l=1;l<=size(B[4]);l++)
              {
                 if(deg(std(B[2]+B[4][l]+MAtmp)[1])!=0)
                 {
//--- the exceptional divisor meets B[2] at the locus of MAtmp
//--- we continue only if the option extra=1 and we have transversal
//--- intersection
                    m=2;
                    if(!transversalTB(B[2],list(B[4][l]),MAtmp))
                    {
                       m=0;break;
                    }
                  }
              }
           }
           if(m)
           {
              if(m==1)
              {
//--- the i-th chart will be marked for deleting because all informations
//--- are already contained in the union of the remaining charts
                 delCharts[i+1]=1;
              }
              else
              {
//--- the option extra=1 and we have transversal intersection
//--- we delete the chart in case of normal crossings
                 if(normalCrossB(B[2],B[4],MAtmp))
                 {
//--- in case of the option extra
//--- the i-th chart will be marked for deleting because all informations
//--- are already contained in the union of the remaining charts

                   delCharts[i+1]=1;
                 }
              }
           }
           if(delCharts[i+1]==0)
           {
              MAtmp[i+1]=MA[i+1];
           }
         }
      }
      for(i=0;i<=s-1;i++)
      {
         if(!delCharts[i+1]){break;}
      }
      if(i==s){delCharts[s]=0;}
   }
   for(i=0;i<=s-1;i++)
   {
     B=ZRes[i+1];
     laM=ZlaM[i+1];
     if(noDel){delCharts[i+1]=0;}
//---keeps chart if the exceptional divisor is not in any other chart
     if((delCharts[i+1])&&(keepDiv))
     {
         for(j=1;j<=size(B[4]);j++)
         {
            if(deg(std(B[4][j])[1])>0)
            {
               x=0;
               for(l=0;l<=s-1;l++)
               {
                  if((l!=i)&&(!delCharts[l+1])&&(deg(std(ZRes[l+1][4][j])[1])>0))
                  {
                     x=1;
                     break;
                  }
               }
               if(!x)
               {
                  delCharts[i+1]=0;
//!!!evtl. diese Karten markieren und nicht weiter aufblasen???
                  break;
               }
            }
         }
     }
//---keeps charts if the intersection of 2 divisors is not in any other chart
     if((delCharts[i+1])&&(keepDiv))
     {
         for(j=1;j<=size(B[4])-1;j++)
         {
            for(l=j+1;l<=size(B[4]);l++)
            {
               if(deg(std(B[4][j]+B[4][l])[1])>0)
               {
                  x=0;
                  for(ll=0;ll<=s-1;ll++)
                  {
                     if((ll!=i)&&(!delCharts[ll+1]))
                     {
                        if(deg(std(ZRes[ll+1][4][j]+ZRes[ll+1][4][l])[1])>0)
                        {
                           x=1;
                           break;
                        }
                     }
                  }
                  if(!x)
                  {
                     delCharts[i+1]=0;
                     break;
                  }
               }
            }
            if(!delCharts[i+1]){break;}
         }
     }
//---keeps charts if the intersection of 3 divisors is not in any other chart
     if((delCharts[i+1])&&(keepDiv))
     {
         for(j=1;j<=size(B[4])-2;j++)
         {
            for(l=j+1;l<=size(B[4])-1;l++)
            {
               for(ll=l+1;ll<=size(B[4]);ll++)
               {
                  if(deg(std(B[4][j]+B[4][l]+B[4][ll])[1])>0)
                  {
                     x=0;
                     for(jj=0;jj<=s-1;jj++)
                     {
                        if((jj!=i)&&(!delCharts[jj+1]))
                        {
                           if(deg(std(ZRes[jj+1][4][j]
                              +ZRes[jj+1][4][l]+ZRes[jj+1][4][ll])[1])>0)
                           {
                              x=1;
                              break;
                           }
                        }
                     }
                     if(!x)
                     {
                       delCharts[i+1]=0;
                       break;
                     }
                  }
               }
               if(!delCharts[i+1]){break;}
            }
            if(!delCharts[i+1]){break;}
        }
     }
     if(delCharts[i+1]==0)
     {
//--- try to decrease the number of variables by substitution
        if((!keepDiv)&&(!noDel))
        {
           list WW=elimpart(B[1]);
           map phiW=basering,WW[5];
           B=phiW(B);
           laM=phiW(laM);
           kill WW;
           kill phiW;
           if(size(B[1])==0)
           {
              LL=0;
              for(j=1;j<=size(B[4]);j++)
              {
                 MM=std(B[4][j]);
                 if(deg(MM[1])>0){LL=LL,MM;}
              }
              LL=variables(LL);
              M=elimpart(B[2]);
              if((size(M[2])!=0)&&(size(std(LL+M[2]))==size(M[2])+size(LL)))
              {
                psi=S,M[5];
                B=psi(B);
                laM=psi(laM);
              }
           }
         }
//---- interreduce B[1],B[2] and all B[4][j]
        B[1]=interred(B[1]);
        B[2]=interred(B[2]);
        E2=B[4];
        for(j=1;j<=size(E2);j++){E2[j]=interred(E2[j]);}
        B[4]=E2;
        v=0;v[size(E2)]=0;
//--- mark those j for which B[4] does not meet B[2]
        for(j=1;j<=size(E2);j++)
        {
            K=E2[j],B[2];
            K=std(K);
            if(deg(K[1])==0)
            {
              v[j]=1;
            }
        }
        B[6]=v;
        B[7]=wvec;
//--- throw away variables which do not occur
        K=B[1],B[2],B[5];          //Aenderung!!!
        for(j=1;j<=size(B[4]);j++){K=K,B[4][j];}
        N=findvars(K);
        if(size(N[1])<n)
        {
           newvar=string(N[1]);
           v=N[4];
           for(j=1;j<=size(v);j++)
           {
              B[5]=subst(B[5],var(v[j]),0);
              laM=subst(laM,var(v[j]),0);
           }
           execute("ring R2=("+charstr(S)+"),("+newvar+"),dp;");
           list BO=imap(S,B);
           ideal laM=imap(S,laM);
        }
        else
        {
           def R2=basering;
           list BO=B;
        }
        ideal JJ=BO[2];
        attrib(JJ,"isEqui",eq);
        attrib(JJ,"isHy",hy);
        BO[2]=JJ;
//--- strict transforms of the known centers
        if(haveCenters)
        {
           ideal tt;
           list tList;
           for(j=1;j<=size(BO[10]);j++)
           {
              tt=std(BO[10][j][1]);
              if(deg(tt[1])>0)
              {
                tt=sat(tt,BO[4][size(BO[4])])[1];
              }
              if((deg(tt[1])>0)&&(deg(std(tt+BO[2]+BO[1])[1])>0))
              {
                 tList[size(tList)+1]=
                            list(tt,BO[10][j][2],BO[10][j][3],BO[10][j][4]);
              }
           }
           BO[10]=tList;
           kill tList;
        }
//--- marking  variables which do not occur in BO[1] and BO[2]
//--- and occur in exactly one BO[4][j], which is the hyperplane given by
//--- this variable
//!!!! not necessarily in exactly one BO[4][j]
        list N=findvars(BO[1]+BO[2]);
        if(size(N[1])<nvars(basering))
        {
           v=N[4];
           if(defined(H)){kill H;}
           if(defined(EE)){kill EE;}
           if(defined(vv)){kill vv;}
           list EE;
           intvec vv;
           ideal H=maxideal(1);
           for(j=1;j<=size(v);j++)
           {
             H[v[j]]=0;
           }
           H=std(H);
           for(l=1;l<=size(BO[4]);l++)
           {
              if(BO[6][l]==1){l++;continue;}
              if(size(ideal(matrix(reduce(BO[4][l],H))-BO[4][l]))==0)
              {
//!!! need further cleanup:
//!!! this part of the code is no longer used since it did not glue properly
//                 BO[6][l]=2;
                 EE[size(EE)+1]=BO[4][l];
                 vv[size(vv)+1]=l;
              }
           }
           if((size(vv)>dim(std(BO[2])))&&(deg(BO[2][1])>0))
           {
              list BOtemp3=BO;
              BOtemp3[4]=EE;
              intvec ivtemp3;
              ivtemp3[size(BOtemp3[4])]=0;
              BOtemp3[6]=ivtemp3;
              BOtemp3[7][1]=-1;
              list iEtemp3=inters_E(BOtemp3);
              if(iEtemp3[2]>=dim(std(BOtemp3[2])))
              {
                 for(l=2;l<=size(vv);l++)
                 {
                    BO[6][vv[l]]=0;
                 }
              }
              kill BOtemp3,ivtemp3,iEtemp3;
           }
        }
        list thisChart=ideal(0),i;
        export thisChart;

//----------------------------------------------------------------------------
// export the basic object and append the ring to the list of rings
//----------------------------------------------------------------------------
        if(debugBlowUp)
        {
           "----> In BlowUp: Adding a single chart";
           "BO:";
           BO;
        }
        if(locaT)
        {
           map locaPhi=R0,laM;
           poly @p=locaPhi(pp);
           export(@p);
        }
        ideal lastMap=laM;
        export lastMap;
        if(invS){list invSat=imap(R0,invSat);export invSat;}
        if(defined(@invmat)){BO[9]=@invmat;}
        if(shortC){list shortcut=imap(R0,shortcut);export(shortcut);}
        export BO;
        result[size(result)+1]=R2;
        setring S;
        kill R2;
     }
   }
   setring R0;
   return(result);
}
example
{"EXAMPLE:";
   echo = 2;
   ring R=0,(x,y),dp;

   ideal W;
   ideal J=x2-y3;
   intvec b=1;
   list E;
   ideal abb=maxideal(1);
   intvec v;
   intvec w=-1;
   matrix M;
   intvec ma;
   list BO=W,J,b,E,abb,v,w,M,ma;

   ideal C=CenterBO(BO)[1];

   list blow=blowUpBO(BO,C,0);
   def Q=blow[1];
   setring Q;
   BO;
}
//////////////////////////////////////////////////////////////////////////////
static proc slocusE(ideal i)
"Internal procedure - no help and no example available
"
{
//--- do slocus in equidimensional case directly -- speed up
  if(size(i)==0){return(ideal(1));}
  if( typeof(attrib(i,"isEqui"))=="int" )
  {
     if(attrib(i,"isEqui")==1)
     {
        ideal j=std(i);
        if(deg(j[1])==0){return(j);}
        int cod  = nvars(basering)-dim(j);
        i        = i+minor(jacob(i),cod);
        return(i);
     }
  }
  return(slocus(i));
}
///////////////////////////////////////////////////////////////////////////////
proc inters_E(list BO)
"USAGE:  inters_E(BO);
@*       BO = basic object, a list: ideal W,
@*                                  ideal J,
@*                                  intvec b,
@*                                  list Ex,
@*                                  ideal ab,
@*                                  intvec v,
@*                                  intvec w,
@*                                  matrix M
ASSUME:  R  = basering, a polynomial ring, W an ideal of R,
@*       J  = ideal containing W,
@*       BO in the setting of case 2 of [Bravo,Encinas,Villamayor]
@*             BO[4]=E, BO[4][1..count]=E^-
@*             BO[7][1]=count
COMPUTE: (W,(P,1),E^+) in the notation of [Bravo,Encinas,Villamayor]
RETURN:  a list l ,
         l[1]: P = product of ideals I(H_i1)+..+I(H_in) over all
                   n-tuples of indices i1..in from 1..count
         l[2]: n = maximal number of H_i from E^- meeting J simultaneously
         l[3]: maximal number of H_i from E meeting J simultaneously
EXAMPLE: internal procedure - no example available
"
{
//---------------------------------------------------------------------
// Initialization
//---------------------------------------------------------------------
  int kk,jj,ii,updated,n,count2,kkdiff;
  def rb=basering;
  def W=BO[1];
  ideal J=BO[1],BO[2];
  int nonnormal;
  int maxkk=dim(std(J));
  int dimJ=maxkk;
  ideal test2;
  list merklist1,merklist2;
  if(size(BO[4])==0)
  {
    list retlist=BO[2],n;
    return(retlist);
  }
  def E=BO[4];
  intvec stoplist=BO[6];
//--- fill in all known information about exceptional divisors not meeting
//--- current chart
  for(ii=1;ii<=size(E);ii++)
  {
    if(deg(std(E[ii])[1])==0)
    {
      stoplist[ii]=1;
    }
  }

  int count=BO[7][1];
  if(!defined(debug_Inters_E))
  {
    int debug_Inters_E=0;
  }
//---------------------------------------------------------------------
// we only want to look at E^-, not at all of E
//---------------------------------------------------------------------
  if (count>-1)
  {
    if (count>0)
    {
      list E_new=E[1..count];
      count2=size(E);
    }
    else
    {
      list E_new;
      count2=size(E);
    }
  }
  else
  {
    list E_new=E;
    count=size(E_new);
    count2=count;
  }
//---------------------------------------------------------------------
// combinatorics is expensive in an interpreted language,
// we leave it to the kernel by translating it into monomial
// ideals in a new ring with variables t(i)
//---------------------------------------------------------------------
  string rstr="ring rcomb=0,(t(1.." + string(size(E)) + ")),dp;";
  execute(rstr);
  ideal potid,potid2;
  list monlist,comblist,merkmon;
  for(kk=1;kk<=count;kk++)
  {
    if(stoplist[kk]==0)
    {
//**************************************************************************/
// it does not make sense to intersect twice by the same E_i
// ===> reduce by t(i)^2
//**************************************************************************/
      potid=potid,t(kk)^2;
    }
    else
    {
//**************************************************************************/
// it does not make sense to consider E_i with v[i]==1 ===> reduce by t(i)
//**************************************************************************/
      potid=potid,t(kk);
//**************************************************************************/
// if stoplist[kk]==2 then J and all E_i automatically intersect E_kk
// hence we need not test it, but we have to lower maxkk by one
//**************************************************************************/
      if(stoplist[kk]==2)
      {
        maxkk=maxkk-1;
        kkdiff++;             // count these for dimension check later on

      }
    }
  }

  potid2=std(potid);
  if(count2>count)
  {
    potid=potid,t((count+1)..count2);
    for(kk=max(1,count);kk<=count2;kk++)
    {
      potid2=potid2,t(kk)^2;
    }
  }
  potid=std(potid);
  potid2=std(potid2);
  for(kk=1;(((kk<=count)||(kk<=maxkk+1))&&(kk<=count2));kk++)
  {
//-------------------------------------------------------------------------
// monlist[kk]=lists of kk entries of E_new, not containing an E_i twice,
// not containing an E_i where v[i]==1
//-------------------------------------------------------------------------
    monlist[kk]=redMax(kk,potid);
//*************************************************************************/
// in the case of n<=maxkk we also need to know whether n would still be
// below this bound if we considered all of E instead of E_new
// ===> merkmon contains previously ignored tuples E_i1,..,E_im
//*************************************************************************/
    if(kk<=maxkk+1)
    {
      merkmon[kk]=redMax(kk,potid2);
      merkmon[kk]=simplify(reduce(merkmon[kk],std(monlist[kk])),2);
    }
  }
  if(debug_Inters_E)
  {
    "----> In Inters_E: the tuples";
    "tuples of E^-:";
    monlist;
    "the remaining tuples:";
    merkmon;
  }
//-------------------------------------------------------------------------
// check whether there is a kk-tuple of E_i intersecting J,
// kk running from 1 to count
//-------------------------------------------------------------------------
  for(kk=1;kk<=count;kk++)
  {
    if(size(monlist[kk])==0) break;
    kill comblist;
    list comblist;
//--- transscribe the tuples from monomial notation to intvec notation
    for(jj=1;jj<=ncols(monlist[kk]);jj++)
    {
      comblist[jj]=leadexp(monlist[kk][jj]);
    }
    setring rb;
    updated=0;
//------------------------------------------------------------------------
// Do the intersections
//------------------------------------------------------------------------
    for(jj=1;jj<=size(comblist);jj++)
    {
//--- jj-th tuple from list of tuples of kk E_i
      test2=J;
      for(ii=1;ii<=count;ii++)
      {
        if(comblist[jj][ii]==1)
        {
          test2=test2,E_new[ii];
        }
      }
      test2=std(test2);
//--- check whether this intersection is non-empty and store it accordingly
      if(deg(test2[1])!=0)
      {
//--- it is non-empty
        if(updated!=0)
        {
          merklist1[size(merklist1)+1]=comblist[jj];
        }
        else
        {
          kill merklist1;
          list merklist1;
          merklist1[1]=comblist[jj];
          updated=1;
          n=kk;
        }
        if(dim(test2)!=maxkk-kk+kkdiff)
        {
          nonnormal=1;
        }
      }
      else
      {
//--- it is empty
        merklist2[size(merklist2)+1]=jj;
      }
    }
    setring rcomb;
    ideal redid;
//---------------------------------------------------------------------
// update monlist and merkmon by the knowledge what intersections are
// empty in the kk-th step
//---------------------------------------------------------------------
    for(jj=1;jj<=size(merklist2);jj++)
    {
      redid=redid,monlist[kk][merklist2[jj]];
    }
    for(jj=kk+1;jj<=count;jj++)
    {
      monlist[jj]=simplify(reduce(monlist[jj],std(redid)),2);
      if(jj<=maxkk+1)
      {
        merkmon[jj]=simplify(reduce(merkmon[jj],std(redid)),2);
      }
    }
    kill redid;
    kill merklist2;
    list merklist2;
  }
  if(debug_Inters_E)
  {
    "----> In Inters_E: intersections found:";
    merklist1;
  }
//---------------------------------------------------------------------
// form the union of the intersections of the appropriate E_i
//---------------------------------------------------------------------
  setring rb;
  ideal center,dummy;
  list centlist;
  for(kk=1;kk<=size(merklist1);kk++)
  {
    for(jj=1;jj<=size(merklist1[kk]);jj++)
    {
      if(merklist1[kk][jj]==1)
      {
        dummy=dummy,E_new[jj];
      }
    }
    if(size(center)==0)
    {
      center=dummy;
      centlist[1]=dummy;
    }
    else
    {
      center=intersect(center,dummy);
      centlist[size(centlist)+1]=dummy;
    }
    dummy=0;
  }
  if(debug_Inters_E)
  {
    "----> In Inters_E: intersection of E_i";
    "maximal number of E_i encountered in:";
    center;
    "the components of this locus:";
    centlist;
    "maximal number of E_i from E^- intersecting simultaneously:",n;
    if(nonnormal)
    {
      "flag nonnormal is set";
    }
  }
  list retlist=center,n;
//-------------------------------------------------------------------------
// If n<=maxkk, then test if this is the case for all of E not just E_new
// using the pairs indicated by merkmon
//-------------------------------------------------------------------------
  int ntotal=n;
  if((n<=maxkk)&&(n<count2)&&(!nonnormal))
  {
//--- check kk-tuples
    for(kk=n+1;kk<=maxkk+1;kk++)
    {
      setring rcomb;
//--- check if there are combinations to be checked
      if(kk>size(merkmon))
      {
        setring rb;
        break;
      }
      if(size(merkmon[kk])!=0)
      {
        kill comblist;
        list comblist;
//--- transscribe tuples from  monomial notation to intvec notation
        for(jj=1;jj<=size(merkmon[kk]);jj++)
        {
          comblist[jj]=leadexp(merkmon[kk][jj]);
        }
        setring rb;
//--- check jj-th tuple from the list of kk-tuples
        for(jj=1;jj<=size(comblist);jj++)
        {
          test2=J;
          for(ii=1;ii<=nvars(rcomb);ii++)
          {
            if(comblist[jj][ii]==1)
            {
              test2=test2,E[ii];
            }
          }
          test2=std(test2);
//--- as soon as we found one we can proceed to the subsequent kk
          if(deg(test2[1])!=0)
          {
            ntotal=kk;
            if(dim(test2)-kkdiff!=maxkk-kk)
            {
              nonnormal=2;
              break;
            }
          }
        }
//--- if we already know that too many E_i intersect simultaneously,
//--- we need not proceed any further
        if(nonnormal)
        {
          break;
        }
      }
      else
      {
        setring rb;
        break;
      }
    }
  }
//-------------------------------------------------------------------------
// update the result accordingly and return it
//-------------------------------------------------------------------------
  if(maxkk<dimJ)
  {
    n=n+dimJ-maxkk;
    ntotal=ntotal+dimJ-maxkk;
  }
  retlist[2]=n;
  retlist[3]=ntotal;
  if(n<=dimJ)
  {
    retlist[4]=centlist;
    retlist[5]=merklist1;
    if(nonnormal)
    {
      retlist[6]=nonnormal;
    }
  }
  return(retlist);
}
///////////////////////////////////////////////////////////////////////////

proc Delta(list BO)
"USAGE:  Delta (BO);
@*       BO = basic object, a list: ideal W,
@*                                  ideal J,
@*                                  intvec b,
@*                                  list Ex,
@*                                  ideal ab,
@*                                  intvec v,
@*                                  intvec w,
@*                                  matrix M
ASSUME:  R  = basering, a polynomial ring, W an ideal of R,
@*       J  = ideal containing W
COMPUTE: Delta-operator applied to J in the notation of
         [Bravo,Encinas,Villamayor]
RETURN:  ideal
EXAMPLE: example Delta;  shows an example
"
{
//---------------------------------------------------------------------------
// Initialization  and sanity checks
//---------------------------------------------------------------------------
   ideal W=BO[1];
   ideal J=BO[2];
   ideal C=simplify(reduce(J,std(W)),2);
   list LC;
   int n=nvars(basering);
//---------------------------------------------------------------------------
// Simple case: W is the empty set
//---------------------------------------------------------------------------
   if(size(W)==0)
   {
      C=C,jacob(J);
      C=std(C);
      return(C);
   }
//---------------------------------------------------------------------------
// General case: W is non-empty
// Step 1: Find a minor of the Jacobian of W which is not identically zero
//         and look at the complement of the zero-set given by this minor;
//         this leads to the system of local parameters
// Step 2: Form the derivatives w.r.t. this system of parameters
//---------------------------------------------------------------------------
//--- Step 1
   list re=findMinor(W);
   list L;
   int ii,i,j,l,k;
   J=C;
   ideal D=ideal(1);
   intvec v,w;
   ideal V;
   poly m;

   for(ii=1;ii<=size(re);ii++)
   {
      C=J;
      L=re[ii];
      matrix A=L[1];  //(1/m)*A is the inverse matrix of the Jacobian of W
                      //corresponding to m
      m=L[2];  //a k- minor of jacob(W), not identically zero
                      //k=n-dim(W)
      V=L[3];  //the elements of W corresponding to m
      v=L[4];  //the indices of variables corresponding to m
      w=L[5];  //the indices of variables not corresponding to m

//--- Step 2
//--- first some initialization depending on results of step 1
      k=size(V);
      matrix dg[1][k];
      matrix df[k][1];
//--- derivatives of the generators of J w.r.t. system of parameters
      for(i=1;i<=size(J);i++)
      {
         for(j=1;j<=n-k;j++)
         {
            for(l=1;l<=k;l++)
            {
               dg[1,l]=diff(V[l],var(w[j]));
               df[l,1]=diff(J[i],var(v[l]));
             }
             C=C,m*diff(J[i],var(w[j]))-dg*A*df;
         }
      }
//--- everything should live in W, not just in the intersection of
//--- D(m) with W
      C=C+W;
      C=sat(C,m)[1];
//--- intersect ideal with previously computed ones to make sure that no
//--- components are lost
      D=intersect(D,C);
      kill dg,df,A;
   }
//--- return minimal set of generators of the result
   list li=mstd(D);
   D=li[2];
   if(size(li[1])<=size(D)){D=li[1];}
   return(D);
}
example
{ "EXAMPLE:";
   echo = 2;
   ring R=0,(x,y,z),dp;

   ideal W=z^2-x;
   ideal J=x*y^2+x^3;
   intvec b=1;
   list E;
   ideal abb=maxideal(1);
   intvec v;
   intvec w=-1;
   matrix M;

   list BO=W,J,b,E,abb,v,w,M;

   Delta(BO);
}

//////////////////////////////////////////////////////////////////////////////
static proc redMax(int k,ideal J)
"Internal procedure - no help and no example available
"
{
//--- reduce maxideal(k) by J, more efficient approach
    int i;
    ideal K=simplify(reduce(maxideal(1),J),2);
    for(i=2;i<=k;i++)
    {
       K=simplify(reduce(K*maxideal(1),J),2);
    }
    return(K);
}

//////////////////////////////////////////////////////////////////////////////
static proc findMinor(ideal W)
"Internal procedure - no help and no example available
"
{
//---------------------------------------------------------------------------
// Initialization and sanity checks
//---------------------------------------------------------------------------
   list L;
   intvec v,w;
   ideal Wstd=std(W);
   int n=nvars(basering);   // total number of columns of Jacobian
   int k=n-dim(Wstd);       // size of minors of Jacobian
   int a=size(W);           // total number of rows of Jacobian
   matrix A[k][k];
   list LW=indexSet(a,k);   // set of tuples of k rows
   list LV=indexSet(n,k);   // set of tuples of k columns
   ideal IW,IV;
   int i,j,l,e;
   list re;
//---------------------------------------------------------------------------
// We need to know which minor corresponds to which variable and to which
// generator of W - therefore we cannot use the function minor()!
//---------------------------------------------------------------------------
//--- choose the generators which we want to differentiate
   for(i=1;i<=size(LW);i++)
   {
      IW=0;
      for(l=1;l<=a;l++)
      {
         if(LW[i][l]!=0){IW[size(IW)+1]=W[l];}
      }
//--- choose the variables by which to differentiate and apply diff
      for(j=1;j<=size(LV);j++)
      {
          IV=0;v=0;w=0;
          for(l=1;l<=n;l++)
          {
             if(LV[j][l]!=0)
             {
                v[size(v)+1]=l;
                IV[size(IV)+1]=var(l);
             }
             else
             {
                w[size(w)+1]=l;

             }
          }
          A=diff(IV,IW);      // appropriate submatrix of Jacobian
//--- if the minor is non-zero, then it might be the one we need
//--- ==> put it in the list of candidates
          if(det(A)!=0)
          {
            v=v[2..size(v)];  // first entry is zero for technical reasons
            w=w[2..size(w)];  // first entry is zero for technical reasons
            L=inverse_L(A);
            L[3]=IW;
            L[4]=v;
            L[5]=w;
            re[size(re)+1]=L;
          }
      }
   }
//---------------------------------------------------------------------------
// return the result
//---------------------------------------------------------------------------
   return(re);
}

/////////////////////////////////////////////////////////////////////////////
static proc indexSet(int a, int b)
"Internal procedure - no help and no example available
"
{
//---------------------------------------------------------------------------
// Find all tuples of size b containing pairwise distict elements from a
// list of a elements
//---------------------------------------------------------------------------
//**************************************************************************/
// Combinatorics is expensive in an interpreted language
// ==> shift it into the kernel
//**************************************************************************/
   def R=basering;
   list L;
   ring S=2,x(1..a),dp;
   ideal I=maxideal(b);
   int i;
   ideal J=x(1)^2;
   for(i=2;i<=a;i++){J=J,x(i)^2;}
   attrib(J,"isSB",1);
   I=reduce(I,J);
   I=simplify(I,2);
   for(i=1;i<=size(I);i++){L[i]=leadexp(I[i]);}
   setring R;
   return(L);
}

/////////////////////////////////////////////////////////////////////////////

proc DeltaList(list BO)
"USAGE:  DeltaList (BO);
@*       BO = basic object, a list: ideal W,
@*                                  ideal J,
@*                                  intvec b,
@*                                  list Ex,
@*                                  ideal ab,
@*                                  intvec v,
@*                                  intvec w,
@*                                  matrix M
ASSUME:  R  = basering, a polynomial ring, W an ideal of R,
@*       J  = ideal containing W
COMPUTE: Delta-operator iteratively applied to J in the notation of
         [Bravo,Encinas,Villamayor]
RETURN:  list l of length ((max w-ord) * b),
         l[i+1]=Delta^i(J)
EXAMPLE: example DeltaList;  shows an example
"
{
//----------------------------------------------------------------------------
// Iteratively apply proc Delta
//----------------------------------------------------------------------------
   int i;
   list L;
   ideal C=BO[2];
   while(deg(C[1])!=0)
   {
      L[size(L)+1]=C;
      C=Delta(BO);
      BO[2]=C;
   }
   return(L);
}
example
{
   "EXAMPLE:";
   echo = 2;
   ring R=0,(x,y,z),dp;

   ideal W=z^2-x;
   ideal J=x*y^2+x^3;
   intvec b=1;
   list E;
   ideal abb=maxideal(1);
   intvec v;
   intvec w=-1;
   matrix M;

   list BO=W,J,b,E,abb,v,w,M;

   DeltaList(BO);
}

/////////////////////////////////////////////////////////////////////////////
static proc deleteInt(intvec v,int i,int ini)
{
//!!! Should be in kernel of Singular
//--- delete i-th entry in intvec v,
//--- if necessary reinitializing v with value ini
   int s=size(v);
   intvec w;
   if((i<s)&&(i>1)){w=v[1..i-1],v[i+1..s];}
   if(s==1){w=ini;return(w);}
   if(i==1){w=v[2..s];}
   if(i==s){w=v[1..s-1];}
   return(w);
}
/////////////////////////////////////////////////////////////////////////////
static proc concatInt(intmat A, intmat B)
{
//!!! Should be in kernel of Singular
//--- concatenate two intmats
  if(nrows(A)!=nrows(B)){ERROR("could not concat, wrong number of rows");}
  intmat tempmat[nrows(A)][ncols(A)+ncols(B)];
  tempmat[1..nrows(A),1..ncols(A)]=A[1..nrows(A),1..ncols(A)];
  tempmat[1..nrows(A),ncols(A)+1..ncols(tempmat)]=B[1..nrows(A),1..ncols(B)];
  return(tempmat);
}

/////////////////////////////////////////////////////////////////////////////
proc CenterBO(list BO,list #)
"USAGE:  CenterBO(BO);
@*       BO = basic object, a list: ideal W,
@*                                  ideal J,
@*                                  intvec b,
@*                                  list Ex,
@*                                  ideal ab,
@*                                  intvec v,
@*                                  intvec w,
@*                                  matrix M
ASSUME:  R  = basering, a polynomial ring, W an ideal of R,
@*       J  = ideal containing W
COMPUTE: the center of the next blow-up of BO in the resolution algorithm
         of [Bravo,Encinas,Villamayor]
RETURN:  list l,
         l[1]: ideal describing the center@*
         l[2]: intvec w obtained in the process of determining l[1]@*
         l[3]: intvec b obtained in the process of determining l[1]@*
         l[4]: intvec inv obtained in the process of determining l[1]
EXAMPLE: example CenterBO;  shows an example
"
{
//---------------------------------------------------------------------------
// Initialization  and sanity checks
//---------------------------------------------------------------------------
   int i,bo7save;
   intvec tvec;
//--- re=center,E^- indices, b vector, n vector
   list re=ideal(1),BO[7],BO[3],tvec;
   ideal J=BO[2];
   if(size(J)==0)
   {
     re[1]=ideal(0);
     return(re);
   }
//--- find Delta^(b-1)(J)
   if(size(reduce(J,std(BO[1])))!=0)
   {
     list L=DeltaList(BO);
   }
   else
   {
     list L;
     L[1]=J;
   }
   if(!defined(debugCenter))
   {
     int debugCenter;
   }
   if(debugCenter)
   {
     "----> In Center: after DeltaList";
     "W";
     BO[1];
     "J";
     BO[2];
     "The Delta List:";
     L;
   }
   int b=size(L);
   if(b==0)
   {
//--- if J=W, we do not need to do anything
//--- returning center=1 marks this chart as completed
     return(re);
   }
//---------------------------------------------------------------------------
// check whether max w-ord is constant
//---------------------------------------------------------------------------
   if(b==BO[3][1])
   {
//--- max w-ord is constant
     if(BO[7][1]==-1)
     {
//--- first got its value in the previous step ==> initialize BO[7]
       tvec[1]=size(BO[4])-1;
       for(i=2;i<=size(BO[7]);i++)
       {
         tvec[i]=BO[7][i];
       }
       re[2]=tvec;
       BO[7]=tvec;
     }
   }
   else
   {
//--- max w-ord changed ==> reset BO[7], correct BO[3]
     tvec[1]=-1;
     re[2]=tvec;
     BO[7]=tvec;
     tvec[1]=b;
     BO[3]=tvec;
     if(defined(invSat))
     {
        invSat[2]=intvec(0);
     }
   }
   re[3]=BO[3];
//---------------------------------------------------------------------------
// reduce from case 2 to case 1 of [Bravo, Encinas, Villamayor]
//---------------------------------------------------------------------------
   ideal C=L[b];
   BO[2]=C;
   if(debugCenter)
   {
     "----> In Center: before intersection with E_i:";
     "bmax:",b;
     "Sing(J,bmax):";
      C;
     "E:";
      BO[4];
     "list marking a priori known intersection properties:",BO[6];
     "index of last element of E^- in E:",BO[7][1];
   }
//--- is intermediate result in iteration already good?
//--- return it to calling proc CenterBO
   if(size(#)>0)
   {
     if(#[1]==2)
     {
       re[1]=C;
       kill tvec;
       intvec tvec=re[2][1];
       re[2]=tvec;
       kill tvec;
       intvec tvec=re[3][1];
       re[3]=tvec;
       kill tvec;
       intvec tvec;
       re[4]=tvec;
       return(re);
     }
   }
//--- do the reduction to case 1
   list E=inters_E(BO);
//--- if J is smooth and not too many E_i intersect simultaneously, let us
//--- try to drop redundant components of the candidate for the center
   if((b==1)&&(size(E)>3))
   {
//--- if J is not smooth we do not want to drop any information
     if((size(E[4])>0) && (dim(std(slocusE(BO[2])))<0))
     {
//--- BO[2]==J because b==1
//--- DropRedundant is the counterpart to DropCoeff
//--- do not leave out one of them separately!!!
      E=DropRedundant(BO,E);
      if(size(E)==1)
       {
         kill tvec;
         intvec tvec=re[2][1];
         re[2]=tvec;
         tvec[1]=re[3][1];
         re[3]=tvec;
         tvec[1]=re[4][1];
         re[4]=tvec;
         re[1]=E[1];
         return(re);
       }
     }
   }
//--- set n correctly
   if(E[2]<BO[9][1])
   {
//--- if n dropped, the subsequent Coeff-object will not be the same
//--- ===> set BO[3][2] to zero to make sure that no previous data is used
      if(defined(tvec)) {kill tvec;}
      intvec tvec=BO[7][1],-1;
      BO[7]=tvec;
      tvec=BO[3][1],0;
      BO[3]=tvec;
      if(defined(invSat))
      {
         invSat[2]=intvec(0);
      }
   }
   re[4][1]=E[2];
   C=E[1]^b+J;
   C=mstd(C)[2];
   ideal C1=std(ideal(L[b])+E[1]);
   if(debugCenter)
   {
     "----> In Center: reduction of case 2 to case 1";
     "Output of inters_E, after dropping redundant components:";
      E;
     "result of intersection with E^-, i.e.(E^-)^b+J:";
     C;
     "candidate for center:";
     C1;
   }
//---------------------------------------------------------------------------
// Check whether we have a hypersurface component
//---------------------------------------------------------------------------
   if(dim(C1)==dim(std(BO[1]))-1)
   {
     if((size(reduce(J,C1,5))==0)&&(size(reduce(C1,std(J),5))==0))
     {
//--- C1 equals J and is of codimension 1 in W
        re[1]=C1;
     }
     else
     {
//--- C1 has a codimension 1 (in W) component
        re[1]=std(equiRadical(C1));
     }
     kill tvec;
     intvec tvec=re[2][1];
     re[2]=tvec;
     tvec[1]=re[3][1];
     re[3]=tvec;
     tvec[1]=re[4][1];
     re[4]=tvec;
//--- is the codimension 1 component a good choice or do we need to reset
//--- the information from the previous steps
     if(transversalT(re[1],BO[4]))
     {
        if(size(E)>2)
        {
          if(E[3]>E[2])
          {
            if(defined(shortcut)){kill shortcut;}
            list shortcut=ideal(0),size(BO[4]),BO[7];
            export(shortcut);
          }
        }
        return(re);
     }

     ERROR("reset in Center, please send the example to the authors.");
   }
//---------------------------------------------------------------------------
// Check whether it is a single point
//---------------------------------------------------------------------------
   if(dim(C1)==0)
   {
      C1=std(radical(C1));
      if(vdim(C1)==1)
      {
//--- C1 is one point
        re[1]=C1;
        kill tvec;
        intvec tvec=re[2][1];
        re[2]=tvec;
        kill tvec;
        intvec tvec=re[3][1];
        re[3]=tvec;
        return(re);
      }
    }
//---------------------------------------------------------------------------
// Prepare input for forming the Coeff-Ideal
//---------------------------------------------------------------------------
   BO[2]=C;
   if(size(BO[2])>5)
   {
      BO[2]=mstd(BO[2])[2];
   }
//--- drop leading entry of BO[3]
   tvec=BO[3];
   if(size(tvec)>1)
   {
     tvec=tvec[2..size(tvec)];
     BO[3]=tvec;
   }
   else
   {
     BO[3][1]=0;
   }
   tvec=BO[9];
   if(size(tvec)>1)
   {
     tvec=tvec[2..size(tvec)];
     BO[9]=tvec;
   }
   else
   {
     BO[9][1]=0;
   }
   bo7save=BO[7][1];          // original value needed for result
   if(defined(shortcut))
   {
     if((bo7save!=shortcut[3][1])&&(size(shortcut[3])!=1))
     {
       kill shortcut;
     }
     else
     {
       shortcut[2]=shortcut[2]-bo7save;
       tvec=shortcut[3];
       if(size(tvec)>1)
       {
         tvec=tvec[2..size(tvec)];
         shortcut[3]=tvec;
       }
       else
       {
         shortcut[3]=intvec(shortcut[2]);
       }
     }
   }
   if(BO[7][1]>-1)
   {
//--- drop E^- and the corresponding information from BO[6]
     for(i=1;i<=BO[7][1];i++)
     {
       BO[4]=delete(BO[4],1);
       intvec bla1=BO[6];
       BO[6]=intvec(bla1[2..size(bla1)]);
       kill bla1;
     }
//--- drop leading entry of BO[7]
     tvec=BO[7];
     if(size(tvec)>1)
     {
       tvec=tvec[2..size(tvec)];
       BO[7]=tvec;
     }
     else
     {
       BO[7][1]=-1;
     }
   }
   else
   {
     if(BO[7][1]==-1)
     {
       list tplist;
       BO[4]=tplist;
       kill tplist;
     }
   }
   if(debugCenter)
   {
     "----> In Center: Input to Coeff";
     "b:",b;
     "BO:";
     BO;
   }
//--- prepare the third entry of the invariant tuple
   int invSatSave=invSat[2][1];
   tvec=invSat[2];
   if(size(tvec)>1)
   {
     tvec=tvec[2..size(tvec)];
     invSat[2]=tvec;
   }
   else
   {
     invSat[2][1]=0;
   }
//---------------------------------------------------------------------------
// Form the Coeff-ideal, if possible and useful; otherwise use the previous
// candidate for the center
//---------------------------------------------------------------------------
   list BO1=Coeff(BO,b);
   if(debugCenter)
   {
     "----> In Center: Output of Coeff";
     BO1;
   }
//--- Coeff returns int if something went wrong
   if(typeof(BO1[1])=="int")
   {
      if(BO1[1]==0)
      {
//--- Coeff ideal was already resolved
        re[1]=C1;
        return(re);
      }
      else
      {
//--- no global hypersurface found
        re=CoverCenter(BO,b,BO1[2]);
        kill tvec;
        intvec tvec=invSatSave;
        for(i=1;i<=size(invSat[2]);i++)
        {
           tvec[i+1]=invSat[2][i];
        }
        invSat[2]=tvec;
        return(re);
      }
   }
   int coeff_invar;
   ideal Idropped=1;
//--- if b=1 drop redundant components of the Coeff-ideal
   if(b==1)
   {
//--- Counterpart to DropRedundant -- do not leave out one of them separately
     Idropped=DropCoeff(BO1);         // blow-up in these components
                                      // is unnecessary
   }
//--- to switch off DropCoeff, set Idropped=1;
   BO1[2]=sat(BO1[2],Idropped)[1];
   if(deg(BO1[2][1])==0)
   {
//--- Coeff ideal is trivial
     C1=radical(C1);
     ideal C2=sat(C1,Idropped)[1];
     if(deg(std(C2)[1])!=0)
     {
        C1=C2;
     }
//Aenderung: Strategie: nur im Notfall ganze except. Divisoren
     if(deg(std(BO1[2])[1])==0)
     {
       list BOtemp=BO;
       int bo17save=BO1[7][1];
       BOtemp[7]=0;
       BO1=Coeff(BOtemp,b,int(0));
       BO1[2]=sat(BO1[2],Idropped)[1];
       if(deg(std(BO1[2])[1])==0)
       {
//--- there is really nothing left to do for the Coeff ideal
//--- the whole original BO1[2], i.e. Idropped, is the upcoming center
          re[1]=Idropped;
          re[2]=intvec(bo7save);
          re[3]=intvec(b);
          re[4]=intvec(E[2]);
          return(re);
       }
       if(deg(std(slocus(radical(BO1[2])))[1])==0)
       {
          re[1]=BO1[2];
//          re[2]=intvec(bo7save,BO1[7][1]);
          re[2]=intvec(bo7save,bo17save);
          re[3]=intvec(b,1);
          re[4]=intvec(E[2],1);
          invSat[2]=intvec(1,0);
          return(re);
       }
//!!! effizienter machen???
       list pr=primdecGTZ(BO1[2]);
       ideal Itemp1=1;
       int aa,bb;
       for(aa=1;aa<=size(pr);aa++)
       {
          if(dim(std(pr[aa][2])) < (dim(std(BO1[1]))-1))
          {
//--- drop components which are themselves exceptional diviosrs
             Itemp1=intersect(Itemp1,pr[aa][1]);
          }
       }
       if(deg(std(Itemp1)[1])!=0)
       {
//--- treat the remaining components of the weak Coeff ideal
          BO1[2]=Itemp1;
       }
       BO1[7]=BO[7];
       for(aa=1;aa<=size(BO1[4]);aa++)
       {
         if(deg(std(BO1[4][aa])[1])==0){aa++;continue;}
         if(defined(satlist)){kill satlist;}
         list satlist=sat(BO1[2],BO1[4][aa]+BO1[1]);
         if(deg(std(satlist[1])[1])==0)
         {
            coeff_invar++;
            if(satlist[2]!=0)
            {
              for(bb=1;bb<=satlist[2]-1;bb++)
              {
                 BO1[2]=quotient(BO1[2],BO1[4][aa]+BO1[1]);
              }
            }
            else
            {
              ERROR("J of temporary object had unexpected value;
                     please send this example to the authors.");
            }
         }
         else
         {
            BO1[2]=satlist[1];
         }
       }
       if(deg(std(Itemp1)[1])==0)
       {
          re[1]=BO1[2];
          re[2]=intvec(bo7save,BO1[7][1]);
          re[3]=intvec(b,1);
          re[4]=intvec(E[2],1);
          invSat[2]=intvec(1,0);
          return(re);
       }
       kill aa,bb;
     }
   }
   if(invSatSave<coeff_invar)
   {
     invSatSave=coeff_invar;
   }
//---------------------------------------------------------------------------
// Determine Center of Coeff-ideal and use it as the new center
//---------------------------------------------------------------------------
   if(!defined(templist))
   {
     if(size(BO1[2])>5)
     {
        BO1[2]=mstd(BO1[2])[2];
     }
     list templist=CenterBO(BO1,2);
//--- only a sophisticated guess of a good center computed by
//--- leaving center before intersection with the E_i.
//--- whether the guess was good, is stored in 'good'.
//--- (this variant saves charts in cases like the Whitney umbrella)
     list E0,E1;

     ideal Cstd=std(radical(templist[1]));
     int good=((deg(std(slocusE(Cstd))[1])==0)&&(dim(std(BO1[2]))<=2));
//if(defined(satlist)){good=0;}
     if(good)
     {
        for(i=1;i<=size(BO[4]);i++)
        {
           if((deg(BO[4][i][1])>0)&&(size(reduce(BO[4][i],Cstd,5))!=0))
           {
              E0[size(E0)+1]=BO[4][i]+Cstd;
              E1[size(E1)+1]=BO[4][i];
           }
        }
        good=transversalT(Cstd,E1);
        if(good)
        {
           good=normalCross(E0);
        }
     }
     if(good)
     {
        list templist2=CenterBO(BO1,1);
        if(dim(std(templist2[1]))!=dim(Cstd))
        {
           templist[1]=Cstd;
           if(defined(shortcut)){kill shortcut;}
           list shortcut=ideal(0),size(BO1[4]),templist[2];
           export(shortcut);
        }
        else
        {
           templist=templist2;
        }
        kill templist2;
     }
     else
     {
//--- sophisticated guess was wrong, follow Villamayor's approach
        kill templist;
        list templist=CenterBO(BO1,1);
     }
   }
   if((dim(std(templist[1]))==dim(std(BO1[1]))-1)
       &&(size(templist[4])==1))
   {
     if(templist[4][1]==0)
     {
        for(i=1;i<=size(BO1[4]);i++)
        {
           if(size(reduce(templist[1],std(BO1[4][i]),5))==0)
           {
              templist[4][1]=1;
              break;
           }
        }
     }
   }
//!!! subsequent line should be deleted
//if(defined(satlist)){templist[3][1]=BO[3][1];}
   if(debugCenter)
   {
     "----> In Center: Iterated Center returned:";
     templist;
   }
//--------------------------------------------------------------------------
// set up the result and return it
//--------------------------------------------------------------------------
   re[1]=templist[1];
   kill tvec;
   intvec tvec;
   tvec[1]=bo7save;
   for(i=1;i<=size(templist[2]);i++)
   {
     tvec[i+1]=templist[2][i];
   }
   re[2]=tvec;
   if(defined(shortcut))
   {
      shortcut[2]=shortcut[2]+bo7save;
      shortcut[3]=tvec;
   }
   kill tvec;
   intvec tvec;
   tvec[1]=invSatSave;
   for(i=1;i<=size(invSat[2]);i++)
   {
      tvec[i+1]=invSat[2][i];
   }
   invSat[2]=tvec;
   kill tvec;
   intvec tvec;
   tvec[1]=b;
   for(i=1;i<=size(templist[3]);i++)
   {
     tvec[i+1]=templist[3][i];
   }
   re[3]=tvec;
   kill tvec;
   intvec tvec;
   tvec[1]=E[2];
   for(i=1;i<=size(templist[4]);i++)
   {
     tvec[i+1]=templist[4][i];
   }
   re[4]=tvec;

   return(re);
}
example
{  "EXAMPLE:";
   echo = 2;
   ring R=0,(x,y,z),dp;

   ideal W;
   ideal J=x2-y2z2;
   intvec b=1;
   list E;
   ideal abb=maxideal(1);
   intvec v;
   intvec w=-1;
   matrix M;

   list BO=W,J,b,E,abb,v,w,M,v;

   CenterBO(BO);
}
//////////////////////////////////////////////////////////////////////////////
static proc CoverCenter(list BO,int b, ideal Jb)
{
//----------------------------------------------------------------------------
// Initialization
//----------------------------------------------------------------------------
   def R=basering;
   int i,j,k;
   intvec merk,merk2,maxv,fvec;
   list L,ceList,re;
   ceList[1]=ideal(0);
   poly @p,@f;
   ideal K,dummy;
   if(!attrib(BO[2],"isSB"))
   {
     BO[2]=std(BO[2]);
   }
   for(i=1;i<=size(Jb);i++)
   {
      list tempmstd=mstd(slocus(Jb[i]));
      if(size(tempmstd[1])>size(tempmstd[2]))
      {
         dummy=tempmstd[2];
      }
      else
      {
         dummy=tempmstd[1];
      }
      kill tempmstd;
      L[i]=dummy;
      K=K,dummy;
   }
   K=simplify(K,2);
//---------------------------------------------------------------------------
// The intersection of the singular loci of the L[i] is empty.
// Find a suitable open covering of the affine chart, such that a global
// hypersurface can be found in each open set.
//---------------------------------------------------------------------------
   matrix M=lift(K,ideal(1));
   j=1;
   for(i=1;i<=nrows(M);i++)
   {
      if(M[i,1]!=0)
      {
         merk[size(merk)+1]=i;
         fvec[size(merk)]=j;
      }
      if((i-k)==size(L[j]))
      {
        k=i;
        j++;
      }
   }
//--------------------------------------------------------------------------
// Find a candidate for the center in each open set
//--------------------------------------------------------------------------
//--- first entry of merk is 0 by construction of merk
   for(i=2;i<=size(merk);i++)
   {
//--- open set is D(@p)
       @p=K[merk[i]];
//--- hypersurface is V(@f)
       @f=Jb[fvec[i]];
       execute("ring R1=("+charstr(R)+"),(@y,"+varstr(R)+"),dp;");
       poly p=imap(R,@p);
       poly f=imap(R,@f);
       list @ce;
       list BO=imap(R,BO);
       BO[1]=BO[1]+ideal(@y*p-1);
       BO[2]=BO[2]+ideal(@y*p-1);
       for(j=1;j<=size(BO[4]);j++)
       {
          BO[4][j]=BO[4][j]+ideal(@y*p-1);
       }
//--- like usual Coeff, but hypersurface is already known
       list BO1=SpecialCoeff(BO,b,f);
//--- special situation in SpecialCoeff are marked by an error code of
//--- type int
       if(typeof(BO1[1])=="int")
       {
         if(BO1[1]==0)
         {
//--- Coeff ideal was already resolved
           @ce[1]=BO[2];
           @ce[2]=BO[7];
           @ce[3]=BO[3];
         }
         else
         {
           if(BO[3]!=0)
           {
//--- intersections with E do not meet conditions ==> reset
              ERROR("reset in Coeff, please send the example to the autors");
           }
         }
       }
       else
       {
//--- now do the recursion as usual
         @ce=CenterBO(BO1);
       }
//---------------------------------------------------------------------------
// Go back to the whole affine chart and form a suitable union of the
// candidates
//---------------------------------------------------------------------------
//--- pass from open set to the whole affine chart by taking the closure
       @ce[1]=eliminate(@ce[1],@y);
       setring R;
       ceList[i]=imap(R1,@ce);
//--- set up invariant vector and determine maximum value of it
       if(size(ceList[i][3])==size(ceList[i][4]))
       {
          kill merk2,maxv;
          intvec merk2,maxv;
          for(j=1;j<=size(ceList[i][3]);j++)
          {
            merk2[2*j-1]=ceList[i][3][j];
            merk2[2*j]=ceList[i][4][j];
            ceList[i][5]=merk2;
            if(maxv<merk2)
            {
               maxv=merk2;
            }
          }
       }
       else
       {
          ERROR("This situation should not occur, please send the example
                 to the authors.");
       }
       kill R1;
   }
   kill merk2;
   intvec merk2=-2;
//--- form the union of the components of the center with maximum invariant
   for(i=1;i<=size(ceList);i++)
   {
     if(size(reduce(ceList[i][1],BO[2],5))==0)
     {
//--- already resolved ==> ignore
       i++;
       continue;
     }
     if(ceList[i][5]==maxv)
     {
        if(merk2!=ceList[i][2])
        {
//--- E^- not of the same size as before resp. initialization
          if(merk2[1]==-2)
          {
//--- initialization: save size of E^-
            merk2=ceList[i][2];
            re[1]=ceList[i][1];
            re[2]=ceList[i][2];
            re[3]=ceList[i][3];
            re[4]=ceList[i][4];
          }
          else
          {
//--- otherwise ignore
            i++;
            continue;
          }
        }
        else
        {
          re[1]=intersect(re[1],ceList[i][1]);
        }
     }
   }
//--------------------------------------------------------------------------
// Perform last checks and return the result
//--------------------------------------------------------------------------
   if(size(re)!=4)
   {
//--- oops: already resolved in all open sets
     re[1]=BO[2];
     re[2]=-1;
     re[3]=0;
     re[4]=intvec(0);
   }
   return(re);
}
//////////////////////////////////////////////////////////////////////////////
static proc SpecialCoeff(list BO,int b,poly f)
{
//----------------------------------------------------------------------------
// Coeff with given hypersurface -- no checks of the hypersurface performed
//----------------------------------------------------------------------------
   int i,ch;
   int e=int(factorial(b));
   ideal C;
   list L=DeltaList(BO);
   int d=size(L);
//--- set up ideal
   for(i=0;i<b;i++)
   {
      C=C+L[i+1]^(e/(b-i));
   }
//--- move to hypersurface V(Z)
   ideal Z=f;
   C=C,Z;
   BO[1]=BO[1]+Z;
   BO[2]=C;
   for(i=1;i<=size(BO[4]);i++)
   {
      BO[6][i]=0;             // reset intersection indicator
      BO[4][i]=BO[4][i]+Z;    // intersect the E_i
      BO[2]=sat(BO[2],BO[4][i]+BO[1])[1];
                              // "strict transform" of J w.r.t E, not "total"
   }
   return(BO);
}

//////////////////////////////////////////////////////////////////////////////
static proc DropCoeff(list BO)
"Internal procedure - no help and no example available
"
{
//--- Initialization
  int i,j;
  list pr=minAssGTZ(BO[2]);
  ideal I=BO[2];
  ideal Itemp;
  ideal Idropped=1;
//--- Tests
  for(i=1;i<=size(pr);i++)
  {
     if(i>size(pr))
     {
//--- the continue statement does not test the loop condition *sigh*
        break;
     }
     if(deg(std(slocus(pr[i]))[1])!=0)
     {
//--- this component is singular ===> we still need it
        i++;
        continue;
     }
     Itemp=sat(I,pr[i])[1];
     if(deg(std(Itemp+pr[i])[1])!=0)
     {
//--- this component is not disjoint from the other ones ===> we still need it
        i++;
        continue;
     }
     if(!transversalT(pr[i],BO[4]))
     {
//--- this component does not meet one of the remaining E_i transversally
//--- ===> we still need it
        i++;
        continue;
     }
     if(!normalCross(BO[4],pr[i]))
     {
//--- this component is not normal crossing with the remaining E_i
//--- ===> we still need it
        i++;
        continue;
     }
     if(defined(EE)){kill EE;}
     list EE;
     for(j=1;j<=size(BO[4]);j++)
     {
        EE[j]=BO[4][j]+pr[i];
     }
     if(!normalCross(EE))
     {
//--- we do not have a normal crossing situation for this component after all
//--- ===> we still need it
        i++;
        continue;
     }
     Idropped=intersect(Idropped,pr[i]);
     I=Itemp;
  }
  return(Idropped);
}

//////////////////////////////////////////////////////////////////////////////
static proc DropRedundant(list BO,list E)
"Internal procedure - no help and no example available
"
{
//---------------------------------------------------------------------------
// Initialization and sanity checks
//---------------------------------------------------------------------------
  int ii,jj,kkdiff,nonnormal,ok;
  ideal testid,dummy;
  ideal center;
  intvec transverse,dontdrop,zerovec;
  transverse[size(BO[4])]=0;
  dontdrop[size(E[4])]=0;
  zerovec[size(E[4])]=0;
  ideal J=BO[2];
  int dimJ=dim(std(BO[2]));
  list templist;
  if(size(E)<5)
  {
//--- should not occur
    return(E);
  }
  for(ii=1;ii<=BO[7][1];ii++)
  {
     if(BO[6][ii]==2)
     {
       kkdiff++;
     }
  }
  int expDim=dimJ-E[2]+kkdiff;
  if(size(E)==6)
  {
    nonnormal=E[6];
  }
//---------------------------------------------------------------------------
// if dimJ were smaller than E[2], we would not have more than 3 entries in
//    in the list E
// if dimJ is also at least E[3] and nonnormal is 0, we only need to test that
// * the intersection is of the expected dimension
// * the intersections of the BO[4][i] and J are normal crossing
// * the elements of E^+ have no influence (is done below)
//---------------------------------------------------------------------------
  if((E[3]<=dimJ)&&(!nonnormal))
  {
    ideal bla=E[1]+BO[2]+BO[1];
    bla=radical(bla);
    bla=mstd(bla)[2];

    if(dim(std(slocusE(bla)))<0)
    {
      if(transversalT(J,BO[4]))
      {
        ok=1;
        if(E[2]==E[3])
        {
//--- no further intersection with elements from E^+
          for(ii=1;ii<=size(E[4]);ii++)
          {
            if(dim_slocus(BO[2]+E[4][ii])!=-1)
            {
              dontdrop[ii]=1;
            }
          }
          if(dontdrop==zerovec)
          {
            list relist;
            relist[1]=std(J);
            return(relist);
          }
        }
      }
    }
  }
//---------------------------------------------------------------------------
// now check whether the E_i actually occurring in the intersections meet
// J transversally (one by one) and mark those elements of E[4] where it is
// not the case
//---------------------------------------------------------------------------
 if(!ok)
 {
     for(ii=1;ii<=size(E[5]);ii++)
     {
//--- test the ii-th tuple of E[4] resp. its indices E[5]
       for(jj=1;jj<=size(E[5][ii]);jj++)
       {
//--- if E[5][ii][jj]==1, E_jj is involved in E[4][ii]
        if(E[5][ii][jj]==1)
         {
//--- transversality not yet determined
           if(transverse[jj]==0)
           {
             templist[1]=BO[4][jj];
             if(transversalT(BO[2],templist))
             {
               transverse[jj]=1;
             }
             else
             {
               transverse[jj]=-1;
               dontdrop[ii]=1;
             }
           }
           else
           {
//--- already computed transversality
             if(transverse[jj]<0)
             {
               dontdrop[ii]=1;
             }
           }
         }
       }
     }
   }
//---------------------------------------------------------------------------
// if one of the non-marked tuples from E^- in E[4] has an intersection
// of the expected dimension and does not meet any E_i from E^+
// - except the ones which are met trivially - , it should be
// dropped from the list.
// it can also be dropped if an intersection occurs and normal crossing has
// been checked.
//---------------------------------------------------------------------------
  for(ii=1;ii<=size(E[4]);ii++)
  {
//--- if E[4][ii] does not have transversal intersections, we cannot drop it
    if(dontdrop[ii]==1)
    {
      ii++;
      continue;
    }
//--- testing ii-th tuple from E[4]
    testid=BO[1]+BO[2]+E[4][ii];
    if(dim(std(testid))!=expDim)
    {
//--- not expected dimension
      dontdrop[ii]=1;
      ii++;
      continue;
    }
    testid=mstd(testid)[2];

    if(dim(std(slocusE(testid)))>=0)
    {
//--- not smooth, i.e. more than one component which intersect
      dontdrop[ii]=1;
      ii++;
      continue;
    }
//--- if E^+ is empty, we are done; otherwise check intersections with E^+
    if(BO[7][1]!=-1)
    {
      if(defined(pluslist)){kill pluslist;}
      list pluslist;
      for(jj=BO[7][1]+1;jj<=size(BO[4]);jj++)
      {
        dummy=BO[4][jj]+testid;
        dummy=std(dummy);
        if(expDim==dim(dummy))
        {
//--- intersection has wrong dimension
          dontdrop[ii]=1;
          break;
        }
        pluslist[jj-BO[7][1]]=BO[4][jj]+testid;
      }
      if(dontdrop[ii]==1)
      {
        ii++;
        continue;
      }
      if(!normalCross(pluslist))
      {
//--- unfortunately, it is not normal crossing
        dontdrop[ii]=1;
      }
    }
  }
//---------------------------------------------------------------------------
// The returned list should look like the truncated output of inters_E
//---------------------------------------------------------------------------
  list retlist;
  for(ii=1;ii<=size(E[4]);ii++)
  {
    if(dontdrop[ii]==1)
    {
      if(size(center)>0)
      {
        center=intersect(center,E[4][ii]);
      }
      else
      {
        center=E[4][ii];
      }
    }
  }
  retlist[1]=center;
  retlist[2]=E[2];
  retlist[3]=E[3];
  return(retlist);
}
//////////////////////////////////////////////////////////////////////////////
proc transversalT(ideal J, list E,list #)
"Internal procedure - no help and no example available
"
{
//----------------------------------------------------------------------------
// check whether J and each element of the list E meet transversally
//----------------------------------------------------------------------------
   def R=basering;
   if(size(#)>0)
   {
     ideal pp=#[1];
   }
   int i;
   ideal T,M;
   ideal Jstd=std(J);
   ideal Tstd;
   int d=nvars(basering)-dim(Jstd)+1;   // d=n-dim(V(J) \cap hypersurface)
   for(i=1;i<=size(E);i++)
   {
      if(size(reduce(E[i],Jstd,5))==0)
      {
//--- V(J) is contained in E[i]
        return(0);
      }
      T=J,E[i];
      Tstd=std(T);
      d=nvars(basering)-dim(Tstd);
      if(deg(Tstd[1])!=0)
      {
//--- intersection is non-empty
//!!! abgeklemmt, da es doch in der Praxis vorkommt und korrekt sein kann!!!
//!!! wenn ueberhaupt dann -1 zurueckgeben!!!
//         if((d>=4)&&(size(T)>=10)){return(0);}
         M=minor(jacob(T),d,Tstd)+T;
         M=std(M);
         if(deg(M[1])>0)
         {
//--- intersection is not transversal
           if(size(#)==0)
           {
              return(0);
           }
           M=std(radical(M));
           if(size(reduce(pp,M,5))>0){return(0);}
         }
      }
   }
//--- passed all tests
   return(1);
}
///////////////////////////////////////////////////////////////////////////////
static proc transversalTB(ideal J, list E,ideal V)
"Internal procedure - no help and no example available
"
{
//----------------------------------------------------------------------------
// check whether J and each element of the list E meet transversally
//----------------------------------------------------------------------------
   def R=basering;

   int i;
   ideal T,M;
   ideal Jstd=std(J);
   ideal Tstd;
   int d=nvars(basering)-dim(Jstd)+1;   // d=n-dim(V(J) \cap hypersurface)
   for(i=1;i<=size(E);i++)
   {
      if(size(reduce(E[i],Jstd,5))==0)
      {
//--- V(J) is contained in E[i]
        return(0);
      }
      T=J,E[i];
      Tstd=std(T);
      d=nvars(basering)-dim(Tstd);
      if(deg(Tstd[1])!=0)
      {
//--- intersection is non-empty
         if((d>=4)&&(size(T)>=10)){return(0);}
         M=minor(jacob(T),d,Tstd)+T;
         M=std(M+V);
         if(deg(M[1])>0)
         {
            return(0);
         }
      }
   }
//--- passed all tests
   return(1);
}
///////////////////////////////////////////////////////////////////////////////
static proc powerI(ideal I,int n,int m)
{
//--- compute (n!/m)-th power of I, more efficient variant
   int i;
   int mon=1;
   for(i=1;i<=size(I);i++)
   {
     if(size(I[i])>1){mon=0;break;}
   }
   if(mon)
   {
      if(size(reduce(I,std(radical(I[1])),5))<size(I)-1){mon=0;}
   }
   if((mon)&&(size(I)>3))
   {
      int e=int(factorial(n))/m;
      ideal J=1;
      poly p=I[1];
      I=I[2..size(I)];
      ideal K=p^e;
      for(i=1;i<=e;i++)
      {
         J=interred(J*I);
         K=K,(p^(e-i))*J;
      }
      return(K);
   }
   for(i=n;i>1;i--)
   {
      if(i!=m)
      {
         I=I^i;
      }
   }
   return(I);
}

///////////////////////////////////////////////////////////////////////////////
proc Coeff(list BO, int b, list #)
"USAGE:  Coeff (BO);
@*       BO = basic object, a list: ideal W,
@*                                  ideal J,
@*                                  intvec b (already truncated for Coeff),
@*                                  list Ex  (already truncated for Coeff),
@*                                  ideal ab,
@*                                  intvec v,
@*                                  intvec w (already truncated for Coeff),
@*                                  matrix M
@*       b  = integer indication bmax(BO)
ASSUME:  R  = basering, a polynomial ring, W an ideal of R,
@*       J  = ideal containing W
COMPUTE: Coeff-Ideal of BO as defined in [Bravo,Encinas,Villamayor]
RETURN:  basic object of the Coeff-Ideal
EXAMPLE: example Coeff;    shows an example
"
{
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!! TASK: lower dimension by more than one in a single step if possible !!!
//!!!       (improve bookkeeping of invariants in Coeff and Center)       !!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//---------------------------------------------------------------------------
// Initialization  and sanity checks
//---------------------------------------------------------------------------
   int i,k,dummy,errtype;
   int ma=size(BO[4]);
   intvec merk;
   if(!defined(debugCoeff))
   {
     int debugCoeff ;
   }
   ideal C;
   list L;
   if(size(#)!=0)
   {
      if(typeof(#[1])=="ideal")
      {
        L=#;
      }
      else
      {
        ma=#[1];
        L=DeltaList(BO);
      }
   }
   else
   {
      L=DeltaList(BO);
   }

   if(debugCoeff)
   {
     "----> In Coeff: result of DeltaList:";
     L;
   }
   int d=size(L);                   // bmax of BO
   if((debugCoeff)&&(d!=b))
   {
      "!!!!!! Length of DeltaList does not equal second argument !!!!!!";
      "!!!!!! BO might not have been ord ~ 1 or wrong b          !!!!!!";
   }
   //if(b>=6){return(0);}              // b is too big					//!!! Sperre entfernt
   int e=int(factorial(b));          // b of Coeff-Ideal
   if(e==0)
   {
     ERROR( "// integer size too small for forming b! .");
   }
   if(b==0)
   {
     ERROR( "// second argument to Coeff should never be zero."  );
   }
//----------------------------------------------------------------------------
// Form the Coeff-Ideal
// Step 1: choose hypersurface
// Step 2: sum over correct powers of Delta^i(BO[2])
// Step 3: do the intersection
//----------------------------------------------------------------------------
//--- Step 1
   ideal Z;
   poly p;
   for(i=1;i<=ncols(L[d]);i++)
   {
//--- Look for smooth hypersurface in generators of Delta^(bmax-1)(BO[2])
     dummy=goodChoice(BO,L[d][i]);
     if(!dummy)
     {
       Z= L[d][i];
       break;
     }
     else
     {
       if(dummy>1)
       {
          merk[size(merk)+1]=i;
       }
       if(dummy>errtype)
       {
         errtype=dummy;
       }
     }
   }
   if(size(Z)==0)
   {
//--- no suitable element in generators of Delta^(bmax-1)(BO[2])
//--- try random linear combination
      for(k=1;k<=10;k++)
      {
         for(i=2;i<=size(merk);i++)
         {
            p=p+random(-100,100)*L[d][merk[i]];
         }
         dummy=goodChoice(BO,p);
         if(!dummy)
         {
            Z=p;
            break;
         }
         else
         {
            p=0;
         }
      }
      if(dummy)
      {
        for(i=1;i<=size(L[d]);i++)
        {
           p=p+random(-100,100)*L[d][i];
        }
        dummy=goodChoice(BO,p);
        if(!dummy)
        {
//--- found a suitable one
           Z=p;
        }
      }
      if(dummy)
      {
//--- did not find a suitable random linear combination either
         if(dummy>errtype)
         {
           errtype=dummy;
         }
         list retlist=errtype,L[d];
         return(retlist);
      }
   }
   if(debugCoeff)
   {
     "----> In Coeff: Chosen hypersurface";
     Z;
   }
//--- Step 2
   C=Z;
   for(i=0;i<b;i++)
   {
      C=C,powerI(simplify(reduce(L[i+1],std(Z)),2),b,b-i);
   }
   C=interred(C);

   if(debugCoeff)
   {
     "----> In Coeff: J before saturation";
     C;
   }

//--- Step 3
   BO[1]=BO[1]+Z;
   BO[2]=C;

   ideal Cweakold = C;
   ideal Istrict;
   ideal Iweak = C;

   ideal Icontrol = C;
   ideal Ccontrolold = C;
   int counter = 0;
   for(i=1;i<=size(BO[4]);i++)
   {
      BO[6][i]=0;             // reset intersection indicator
      if(BO[4][i] != 1){
		BO[4][i]=BO[4][i]+Z;    // intersect the E_i
     		//geaendert!!!
		//	if(i<=ma)
			//{
        //BO[2]=sat(BO[2],BO[4][i]+BO[1])[1];
                       // "strict transform" of J w.r.t E, not "total"
                       //weak transform of J w.r.t E 
     
				counter = 0;
				Cweakold = Iweak;
				while(EqualityOfIdeal(Iweak *BO[4][i]+BO[1],Cweakold ) == 1 or counter == 0){
					Cweakold = Iweak;
					//Iweak = quotient(Iweak, BO[4][i]+BO[1]);
					Iweak = quotient(Iweak, BO[4][i]);
					counter++;				
				}               //weak transform of J w.r.t E
				Iweak = Cweakold;
				Ccontrolold = Icontrol;
				for(k=1;k<=b ;k++){
					//Icontrol = quotient(Icontrol, BO[4][1]+BO[1]);
					Icontrol = quotient(Icontrol, BO[4][1]);
					if( k < b){
						Ccontrolold = Icontrol;
					}else{
						Icontrol = Ccontrolold;
					}
				}               //controlled transform of J w.r.t E and b
			//}
		}
  }
	Icontrol = C;
  	BO[2] = Iweak;
	//BO[2] = Istrict;
  if(debugCoeff)
   {
     "----> In Coeff:";
     " J after saturation:";
     BO[2];
   }
   return(list(BO,Icontrol));
}
example
{"EXAMPLE:";
   echo = 2;
   ring R=0,(x,y,z),dp;

   ideal W;
   ideal J=z^2+x^2*y^2;
   intvec b=0;
   list E;
   ideal abb=maxideal(1);
   intvec v;
   intvec w=-1;
   matrix M;

   list BO=W,J,b,E,abb,v,w,M;

   Coeff(BO,2);
}
//////////////////////////////////////////////////////////////////////////////
static proc goodChoice(list BO, poly p)
"Internal procedure - no help and no example available
"
{
//---------------------------------------------------------------------------
// test whether new W is smooth
//---------------------------------------------------------------------------
   ideal W=BO[1]+ideal(p);
   if(size(reduce(p,std(BO[1]),5))==0)
   {
//--- p is already in BO[1], i.e. does not define a hypersurface in W
     return(1);
   }
   if(dim(std(slocusE(W)))>=0)
//   if(dim(timeStd(slocusE(W),20))>=0)
   {
//--- new W would not be smooth
     return(1);
   }
   if(size(BO[4])==0)
   {
//--- E is empty, no further tests necessary
     return(0);
   }
//--------------------------------------------------------------------------
// test whether the hypersurface meets the E_i transversally
//--------------------------------------------------------------------------
   list E=BO[4];
   int i,d;
   ideal T=W;
   ideal Tstd=std(T);
   d=nvars(basering)-dim(Tstd)+1;
   ideal M;
   for(i=1;i<=size(E);i++)
   {
      T=W,E[i];
      M=minor(jacob(T),d,Tstd)+T;
      M=std(M);
      if(deg(M[1])>0)
      {
//--- intersection not transversal
        return(2);
      }
   }
//--------------------------------------------------------------------------
// test whether the new E_i have normal crossings
//--------------------------------------------------------------------------
   for(i=1;i<=size(E);i++)
   {
	  if( size(reduce(p,std(E[i]),5))!=0){
		E[i]=E[i],p;
	}
   }
   if(normalCross(E))
   {
     return(0);
   }
   else
   {
     return(2);
   }
}
//////////////////////////////////////////////////////////////////////////////

proc presentTree(list L)
"USAGE:  presentTree(L);
         L=list, output of resolve
RETURN:  nothing, only pretty printing of the output data of resolve()
EXAMPLE: none
"
{
  def r=basering;
  int i,j,k;
  if(size(L[2])==1)
  {
     "The object was already resolved or the list L does not";
     "have required input format. There is just one chart in";
     "the tree.";
     return();
  }
  for(i=1;i<=size(L[1]);i++)
  {
    "                        ";
    "/////////////////////////// Final Chart",i,"/////////////////////////";
    def s=L[1][i];
    setring s;
    "======================== History of this chart ======================";
    for(j=2;j<=ncols(path);j++)
    {
      "                  ";
      "Blow Up",j-1,":";
      "     Center determined in L[2]["+string(path[1,j])+"],";
      "     Passing to chart ",path[2,j]," in resulting blow up.";
    }
    "                 ";
    "======================== Data of this chart ========================";
    showBO(BO);
    setring r;
    kill s;
    pause();
  }
  "////////////////////////////////////////////////////////////////////";
  "For identification of exceptional divisors please use the tools";
  "provided by reszeta.lib, e.g. collectDiv.";
  "For viewing an illustration of the tree of charts please use the";
  "procedure ResTree from resgraph.lib.";
  "////////////////////////////////////////////////////////////////////";
  return();
}
//////////////////////////////////////////////////////////////////////////////

proc showBO(list BO)
"USAGE:  showBO(BO);
@*        BO=basic object, a list: ideal W,
@*                                  ideal J,
@*                                  intvec b (already truncated for Coeff),
@*                                  list Ex  (already truncated for Coeff),
@*                                  ideal ab,
@*                                  intvec v,
@*                                  intvec w (already truncated for Coeff),
@*                                  matrix M
RETURN:   nothing, only pretty printing
EXAMPLE:  none
"
{
  "                       ";
  "==== Ambient Space: ";BO[1];"      ";
  "==== Ideal of Variety: ";BO[2];"      ";
  int i;
  list M;
  for(i=1;i<=size(BO[4]);i++)
  {
    M[i]=ideal(BO[4][i]);
  }
  "==== Exceptional Divisors: ";print(M);"   ";
  "==== Images of variables of original ring:";BO[5];"   ";
}
//////////////////////////////////////////////////////////////////////////////
////////////////////////    main procedure    ////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
proc resolve(ideal J, list #)
"USAGE:  resolve (J); or resolve (J,i[,k]);
@*       J ideal
@*       i,k int
COMPUTE: a resolution of J,
@*       if i > 0 debugging is turned on according to the following switches:
@*       j1: value 0 or 1; turn off or on correctness checks in all steps
@*       j2: value 0 or 2; turn off or on debugCenter
@*       j3: value 0 or 4; turn off or on debugBlowUp
@*       j4: value 0 or 8; turn off or on debugCoeff
@*       j5: value 0 or 16:turn off or on debugging of Intersection with E^-
@*       j6: value 0 or 32:turn off or on stop after pass throught the loop
@*       i=j1+j2+j3+j4+j5+j6
RETURN:  a list l of 2 lists of rings
         l[1][i] is a ring containing a basic object BO, the result of the
         resolution.
         l[2] contains all rings which occurred during the resolution process
NOTE:    result may be viewed in a human readable form using presentTree()
EXAMPLE: example resolve;  shows an example
"
{
//----------------------------------------------------------------------------
// Initialization and sanity checks
//----------------------------------------------------------------------------
   def R=basering;
   list allRings;
   allRings[1]=R;
   list endRings;
   module path=[0,-1];
   ideal W;
   list E;
   ideal abb=maxideal(1);
   intvec v;
   intvec bvec;
   intvec w=-1;
   matrix intE;
   int extra;
   if(defined(BO)){kill BO;}
   if(defined(cent)){kill cent;}

   ideal Jrad=equiRadical(J);
   if(size(reduce(Jrad,std(J),5))!=0)
   {
      "WARNING! The input is not reduced or not equidimensional!";
      "We will continue with the reduced top-dimensional part of input";
      J=Jrad;
   }

   int i,j,debu,loca,locaT,ftemp,debugResolve,smooth;
//--- switches for local and for debugging may occur in any order
   i=size(#);
   extra=3;
   for(j=1;j<=i;j++)
   {
     if(typeof(#[j])=="int")
     {
       debugResolve=#[j];
//--- debu: debug switch for resolve, smallest bit in debugResolve
       debu=debugResolve mod 2;
     }
     else
     {
        if(#[j]=="E"){extra=0;}
        if(#[j]=="A"){extra=2;}
        if(#[j]=="K"){extra=3;}
        if(#[j]=="L"){loca=1;}
     }
   }
   if(loca)
   {
      list qs=minAssGTZ(J);
      ideal K=ideal(1);
      for(j=1;j<=size(qs);j++)
      {
         if(size(reduce(qs[j],maxideal(1),5))==0)
         {
            K=intersect(K,qs[j]);
         }
      }
      J=K;
      list qr=minAssGTZ(slocus(J));
      K=ideal(1);
      for(j=1;j<=size(qr);j++)
      {
         if(size(reduce(qr[j],maxideal(1),5))!=0)
         {
            K=intersect(K,qr[j]);
            smooth++;
         }
         else
         {
            if(dim(std(qr[j]))>0){loca=0;}
//---- test for isolated singularity at 0
         }
      }
      K=std(K);
//---- if deg(K[1])==0 the point 0 is on all components of the singular
//---- locus and we can work globally
      if(smooth==size(qr)){smooth=-1;}
//---- the point 0 is not on the singular locus
      if((deg(K[1])>0)&&(smooth>=0)&&(!loca))
      {
         locaT=1;
         poly @p;
         for(j=1;j<=size(K);j++)
         {
            if(jet(K[j],0)!=0)
            {
               @p=K[j];
               break;
            }
         }
         export(@p);
      }
      if((loca)&&(!smooth)){loca=0;}
//---- the case that 0 is isolated singularity and the only singular point
   }
   export(locaT);
//---In case of option "L" the following holds
//---loca=0 and locaT=0  we perform the global case
//---loca !=0: 0 is isolated singular point, but there are other singularities
//---locaT!=0: O is singular point, but not isolated, and there is a componente//---          of the singular locus not containing 0

//--- if necessary, set the corresponding debugFlags
   if(defined(debugResolve))
   {
//--- 2nd bit from the right
     int debugCenter=(debugResolve div 2) mod 2;
     export debugCenter;
//--- 3rd bit from the right
     int debugBlowUp=(debugResolve div 4) mod 2;
     export debugBlowUp;
//--- 4th bit from the right
     int debugCoeff=(debugResolve div 8) mod 2;
     export debugCoeff;
//--- 5th bit from the right
     int debug_Inters_E=(debugResolve div 16) mod 2;
     export debug_Inters_E;
//--- 6th bit from the right
     int praes_stop=(debugResolve div 32) mod 2;
   }
//--- set the correct attributes to J for speed ups
   if( typeof(attrib(J,"isEqui"))!="int" )
   {
      if(size(J)==1)
      {
         attrib(J,"isEqui",1);
      }
      else
      {
         attrib(J,"isEqui",0);
      }
   }
   if(size(J)==1)
   {
      attrib(J,"isHy",1);
   }
   else
   {
      attrib(J,"isHy",0);
   }
//----------------------------------------------------------------------   
//END OF INIT AND BEGIN OF THE PROCEDURE!!!-----------------------------
//----------------------------------------------------------------------
   
   
//--- create the BO
   list BO=W,J,bvec,E,abb,v,w,intE;
   if(defined(invSat)){kill invSat;}
   list invSat=ideal(0),intvec(0);
   export(invSat);
   
   BO[9]=intvec(0);
   export BO;
   list tmpList;
   int blo;
   int k,Ecount,tmpPtr;
   i=1;
   if(smooth==-1)
   {
      endRings[1]=R;
      list result=endRings,allRings;
      if(debu)
      {
         "============= result will be tested ==========";
         "                                              ";
         "the number of charts obtained:",size(endRings);
         "=============     result is o.k.    ==========";
      }
      kill debugCenter,debugBlowUp,debugCoeff,debug_Inters_E;
      return(result);
   }
   
   
//-----------------------------------------------------------------------------
// While there are rings to be considered, determine center and blow up
//-----------------------------------------------------------------------------
   while(i<size(allRings))
   {
      i++;
      def S=allRings[i];
      setring S;
      list pr;
      ideal Jstd=std(BO[2]);
//-----------------------------------------------------------------------------
// Determine Center
//-----------------------------------------------------------------------------
      if(i==1)
      {
         list deltaL=DeltaList(BO);
         ideal sL=radical(deltaL[size(deltaL)]);
         if((deg(std(slocus(sL))[1])==0)&&(size(minAssGTZ(sL))==1))
         {
            list @ce=sL,intvec(-1),intvec(0),intvec(0);
            ideal cent=@ce[1];
         }
      }
//--- before computing a center, check whether we have a stored one
      if(size(BO)>9)
      {
         while(size(BO[10])>0)
         {
            list @ce=BO[10][1];
//--- check of the center
//--- use stored center
            BO[10]=delete(BO[10],1);
            if(size(@ce[1])==0)
            {
//--- stored center was not ok
              continue;
            }
            tmpPtr=0;
            for(Ecount=1;Ecount <= size(@ce[2]); Ecount++)
            {
              if(@ce[2][Ecount]>-1)
              {
                tmpPtr=tmpPtr+@ce[2][Ecount];
              }
              else
              {
                @ce[2][Ecount]=size(BO[4])-tmpPtr-1;
                for(int cnthlp=1;cnthlp<=size(BO[10]);cnthlp++)
                {
                   BO[10][cnthlp][2][Ecount]=@ce[2][Ecount];
                }
                kill cnthlp;
                break;
              }
            }
            if(Ecount<size(@ce[2]))
            {
              for(tmpPtr=Ecount+1;tmpPtr<=size(@ce[2]);tmpPtr++)
              {
                @ce[2][tmpPtr]=0;
                for(int cnthlp=1;cnthlp<=size(BO[10]);cnthlp++)
                {
                   BO[10][cnthlp][2][tmpPtr]=@ce[2][tmpPtr];
                }
                kill cnthlp;
              }
            }
            break;
         }
         if(defined(@ce))
         {
           if(size(@ce[1])==0)
           {
              kill @ce;
           }
           else
           {
              ideal cent=@ce[1];
           }
         }
         if(size(BO[10])==0)
         {
//--- previously had stored centers, all have been used; we need to clean up
            BO=delete(BO,10);
         }
      }
      if((loca)&&(i==1))
      {
//--- local case: initial step is blow-up in origin
         if(defined(@ce)){kill @ce;}
         if(defined(cent)){kill cent;}
         if(size(reduce(slocusE(BO[2]),maxideal(1),5))==0)
         {
            list @ce=maxideal(1),intvec(-1),intvec(0),intvec(0);
         }
         else
         {
            list @ce=BO[2],intvec(-1),intvec(1),intvec(0);
         }
         ideal cent=@ce[1];
      }
      if(((loca)||(locaT))&&(i!=1))
      {
         int JmeetsE;
         for(j=1;j<=size(BO[4]);j++)
         {
            if(deg(std(BO[2]+BO[4][j])[1])!=0)
            {
               JmeetsE=1;
               break;
            }
         }
         if(!JmeetsE)
         {
            list @ce=BO[2],intvec(-1),intvec(1),intvec(0);
            ideal cent=@ce[1];
         }
         kill JmeetsE;
      }
      if((locaT)&&(!defined(@ce)))
      {
          if(@p!=1)
          {
             list tr=minAssGTZ(slocusE(BO[2]));
             ideal L=ideal(1);
             for(j=1;j<=size(tr);j++)
             {
                if(size(reduce(ideal(@p),std(tr[j]),5))==0)
                {
                   L=intersect(L,tr[j]);
                }
             }
             L=std(L);
             if(deg(L[1])==0)
             {
                @p=1;
             }
             else
             {
               ideal fac=factorize(@p,1);
               if(size(fac)==1)
               {
                 @p=fac[1];
               }
               else
               {
                  for(j=1;j<=size(fac);j++)
                  {
                     if(reduce(fac[j],L)==0)
                     {
                        @p=fac[j];
                        break;
                     }
                  }
               }
             }
             kill tr,L;
          }
          execute("ring R1=("+charstr(S)+"),(@z,"+varstr(S)+"),dp;");
          poly p=imap(S,@p);
          list BO=imap(S,BO);
          list invSat=imap(S,invSat);
          export(invSat);
          ideal te=std(BO[2]);
          BO[1]=BO[1]+ideal(@z*p-1);
          BO[2]=BO[2]+ideal(@z*p-1);
          for(j=1;j<=size(BO[4]);j++)
          {
             BO[4][j]=BO[4][j]+ideal(@z*p-1);
          }
//--- for computation of center: drop components not meeting the Ei
          def BO2=BO;
          list qs=minAssGTZ(BO2[2]);
          ideal K=ideal(1);
          for(j=1;j<=size(qs);j++)
          {
             if(CompMeetsE(qs[j],BO2[4]))
             {
                K=intersect(K,qs[j]);
             }
          }
          BO2[2]=K;
//--- check whether we are done
          if(deg(std(BO2[2])[1])==0)
          {
             list @ce=BO[2],intvec(-1),intvec(1),intvec(0);
          }
          if(!defined(@ce))
          {
                list @ce=CenterBO(BO2);
          }
//--- if computation of center returned BO2[2], we are done
//--- ==> set @ce to BO[2], because later checks work with BO instead of BO2
          if((size(reduce(@ce[1],std(BO2[2]),5))==0)&&
             (size(reduce(BO2[2],std(@ce[1]),5))==0))
          {
             @ce[1]=BO[2];
          }
          if(size(specialReduce(@ce[1],te,p))==0)
          {
             BO=imap(S,BO);
             @ce[1]=BO[2];
          }
          else
          {
             @ce[1]=eliminate(@ce[1],@z);
          }
          setring S;
          list @ce=imap(R1,@ce);
          kill R1;

         if((size(reduce(BO[2],std(@ce[1]),5))==0)
             &&(size(reduce(@ce[1],Jstd,5))==0))
         {
//--- J and center coincide
            pr[1]=@ce[1];
            ideal cent=@ce[1];
         }
         else
         {
//--- decompose center and use first component
            pr=minAssGTZ(@ce[1]);
               if(size(reduce(@p,std(pr[1]),5))==0){"Achtung";~;}
               if(deg(std(slocus(pr[1]))[1])>0){"singulaer";~;}
            ideal cent=pr[1];
         }
         if(size(pr)>1)
         {
//--- store the other components
            for(k=2;k<=size(pr);k++)
            {
               if(size(reduce(@p,std(pr[k]),5))==0){"Achtung";~;}
               if(deg(std(slocus(pr[k]))[1])>0){"singulaer";~;}
               if(size(reduce(@p,std(pr[k]),5))!=0)
               {
                  tmpList[size(tmpList)+1]=list(pr[k],@ce[2],@ce[3],@ce[4]);
               }
            }
            BO[10]=tmpList;
            kill tmpList;
            list tmpList;
         }
      }
      if(!defined(@ce))
      {
//--- no previously determined center, we need to compute one
         if(loca)
         {
//--- local case: center should be inside exceptional locus
            ideal Ex=ideal(1);
            k=0;
            for(j=1;j<=size(BO[4]);j++)
            {
               if(deg(BO[4][j][1])!=0)
               {
                 Ex=Ex*BO[4][j];   //----!!!!hier evtl. Durchschnitt???
                 k++;
               }
            }
//--- for computation of center: drop components not meeting the Ei
            list BOloc=BO;
            list qs=minAssGTZ(BOloc[2]);
            ideal K=ideal(1);
            for(j=1;j<=size(qs);j++)
            {
              if(CompMeetsE(qs[j],BOloc[4]))
              {
                 K=intersect(K,qs[j]);
              }
            }
            BOloc[2]=K;
//--- check whether we are done
            if(deg(std(BOloc[2])[1])==0)
            {
               list @ce=BO[2],intvec(-1),intvec(1),intvec(0);
            }
            if(!defined(@ce))
            {
              if(BO[3][1]!=0)
              {
                 BOloc[2]=BO[2]+Ex^((BO[3][1] div k)+1);//!!!!Vereinfachen???
              }
              else
              {
                 BOloc[2]=BO[2]+Ex^((size(DeltaList(BO)) div k)+1);
              }
              list @ce=CenterBO(BOloc);
              if(size(reduce(Ex,std(@ce[1]),5))!=0)
              {
                 list tempPr=minAssGTZ(@ce[1]);
                 for(k=size(tempPr);k>=1;k--)
                 {
                    if(size(reduce(Ex,std(tempPr[k]),5))!=0)
                    {
                      tempPr=delete(tempPr,k);
                    }
                 }
                 @ce[1]=1;
                 for(k=1;k<=size(tempPr);k++)
                 {
                    @ce[1]=intersect(@ce[1],tempPr[k]);
                 }
                 if(deg(std(@ce[1])[1])==0)
                 {
                    @ce[1]=BO[2];
                 }
              }
            }
//--- test whether we are done
            if(size(reduce(slocusE(BO[2]),std(@ce[1]),5))!=0)
            {
               if(transversalT(BO[2],BO[4]))
               {
                  if(defined(E)){kill E;}
                  list E=BO[4];
                  for(j=1;j<=size(E);j++){if(deg(E[j][1])>0){E[j]=E[j]+BO[2];}}
                  if(normalCross(E))
                  {
                     @ce[1]=BO[2];
                  }
                  kill E;
               }
            }
         }
         else
         {
//--- non-local
            list @ce=CenterBO(BO);
//--- check of the center
            if((size(@ce[1])==0)&&(size(@ce[4])<(size(@ce[3])-1)))
            {
               intvec xxx=@ce[3];
               xxx=xxx[1..size(@ce[4])];
               @ce[3]=xxx;
               xxx=@ce[2];
               xxx=xxx[1..size(@ce[4])];
               @ce[2]=xxx;
               kill xxx;
            }
         }
         if((size(reduce(BO[2],std(@ce[1]),5))==0)
             &&(size(reduce(@ce[1],Jstd,5))==0))
         {
//--- J and center coincide
            pr[1]=@ce[1];
            ideal cent=@ce[1];
         }
         else
         {
//--- decompose center and use first component
            pr=minAssGTZ(@ce[1]);
            ideal cent=pr[1];
         }
         if(size(pr)>1)
         {
//--- store the other components
            for(k=2;k<=size(pr);k++)
            {
               tmpList[k-1]=list(pr[k],@ce[2],@ce[3],@ce[4]);
            }
            BO[10]=tmpList;
            kill tmpList;
            list tmpList;
         }
      }
//--- do not forget to update BO[7] and BO[3]
      export cent;
      BO[7]=@ce[2];
      BO[3]=@ce[3];
      if((loca||locaT)&&(size(@ce)<4)){@ce[4]=0;} //Provisorium !!!
      if((size(@ce[4])<size(@ce[2])-1)||(size(@ce[4])<size(@ce[3])-1))
      {
        if((deg(std(@ce[1])[1])==0)&&(deg(std(BO[2])[1])==0))
        {
           intvec nullvec;
           nullvec[size(@ce[2])-1]=0;
           @ce[4]=nullvec;
           kill nullvec;
        }
        else
        {
           "ERROR:@ce[4] hat falsche Laenge - nicht-trivialer Fall";
           ~;
        }
      }
      if((typeof(@ce[4])=="intvec") || (typeof(@ce[4])=="intmat"))
      {
        BO[9]=@ce[4];
      }
//---------------------------------------------------------------------------
// various checks and debug output
//---------------------------------------------------------------------------
      if((debu) || (praes_stop))
      {
//--- Show BO of this step
         "++++++++++++++ Overview of Current Chart +++++++++++++++++++++++";
         "Current number of final charts:",size(endRings);
         "Total number of charts currently in chart-tree:",size(allRings);
         "Index of the current chart in chart-tree:",i;
         "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++";
         showBO(BO);
         "-------------------------- Upcoming Center ---------------------";
         interred(cent);
         "----------------------------------------------------------------";
      }
      if(praes_stop)
      {
         ~;
      }
      if(debu)
      {
//--- various checks, see output for comments
         if(size(BO[1])>0)
         {
            if(deg(BO[1][1])==0)
            {
               "!!!  W is empty  !!!";
               path;
               setring R;
               kill S;
               list result=endRings,allRings;
               return(result);
            }
            if(deg(std(slocusE(BO[1]))[1])>0)
            {
               "!!!  W not smooth  !!!";
               path;
               setring R;
               kill S;
               list result=endRings,allRings;
               return(result);
            }
         }
         if((!loca)&&(!locaT))
         {
            if(deg(std(slocusE(cent+BO[1]))[1])>0)
            {
               "!!!  Center not smooth  !!!";
               path;
               std(cent+BO[1]);
               ~;
               setring R;
               kill S;
               list result=endRings,allRings;
               return(result);
            }
         }
         for(j=1;j<=size(BO[4]);j++)
         {
            if(deg(BO[4][j][1])>0)
            {
               if(deg(std(slocusE(BO[4][j]+BO[1]))[1])>0)
               {
                  "!!!  exceptional divisor is not smooth  !!!";
                  path;
                  setring R;
                  kill S;
                  list result=endRings,allRings;
                  return(result);
               }
            }
         }
         if((!loca)&&(!locaT))
         {
            if((norC(BO,cent))&&(size(reduce(cent,Jstd,5))!=0))
            {
               "!!!  this chart is already finished  !!!";
               cent=BO[2];
               ~;
            }
         }
      }
//----------------------------------------------------------------------------
// Do the blow up
//----------------------------------------------------------------------------
//!!!! Change this as soon as there is time!!!
//!!!! quick and dirty bug fix for old shortcut which has not yet been killed
if((dim(std(cent))==0)&&defined(shortcut)) {kill shortcut;}
//!!! end of bugfix
      if(size(reduce(cent,Jstd,5))!=0)
      {
//--- center does not equal J
         tmpList=blowUpBO(BO,cent,extra);
         if((debu)&&(!loca)&&(!locaT))
         {
//--- test it, if debu is set
            if(!testBlowUp(BO,cent,tmpList,i,extra))
            {
               "non-redundant chart has been killed!";
               ~;
            }
         }
//--- extend the list of all rings
         allRings[size(allRings)+1..size(allRings)+size(tmpList)]=
                         tmpList[1..size(tmpList)];
         for(j=1;j<=size(tmpList);j++)
         {
            def Q=allRings[size(allRings)-j+1];
            setring Q;
            def path=imap(S,path);
            path=path,[i,size(tmpList)-j+1];
            export path;
            setring S;
            kill Q;
         }
         kill tmpList;
         list tmpList;
      }
      else
      {
//--- center equals J
           k=0;
           for(j=1;j<=size(BO[6]);j++)
           {
              if(BO[6][j]!=1)
              {
//--- there is an E_i which meets J in this chart
                k=1;
                break;
              }
           }
           if((k)||(extra==2))
           {
//--- chart finished, non-redundant
              endRings[size(endRings)+1]=S;
           }
      }
      kill pr;
      setring R;
      kill S;
   }
//---------------------------------------------------------------------------
// set up the result, test it (if debu is set) and return it
//---------------------------------------------------------------------------
   list result=endRings,allRings;
   if(debu)
   {
      "============= result will be tested ==========";
      "                                              ";
      "the number of charts obtained:",size(endRings);
      if(locaT){loca=2;}
      int tes=testRes(J,endRings,loca);
      if(tes)
      {
         "=============     result is o.k.    ==========";
      }
      else
      {
         "============     result is wrong    =========="; ~;
      }
   }
   kill debugCenter,debugBlowUp,debugCoeff,debug_Inters_E;
   if(locaT){kill @p;}
   kill locaT;
   return(result);
}
example
{"EXAMPLE:";
   echo = 2;
   ring R=0,(x,y,z),dp;
   ideal J=x3+y5+yz2+xy4;
   list L=resolve(J,0);
   def Q=L[1][7];
   setring Q;
   showBO(BO);
}
//////////////////////////////////////////////////////////////////////////
//static
proc CompMeetsE(ideal J, list E)
"Internal procedure - no help and no example available
"
{
   int i;
   for(i=1;i<=size(E);i++)
   {
      if(deg(std(E[i])[1])!=0)
      {
         if(deg(std(J+E[i])[1])!=0)
         {
            return(1);
         }
      }
   }
   return(0);
}

//========================================================================
//--------------  procedures for testing the result ----------------------
//                (not yet commented)
//========================================================================

//////////////////////////////////////////////////////////////////////////
static proc testRes(ideal J,list L,int loca)
"Internal procedure - no help and no example available
"
{
   int loc;
   if(defined(locaT)){loc=locaT;}
   if(loc){loca=0;}
   def R=basering;
   ideal M=maxideal(1);
   int i,j,tr;
   for(i=1;i<=size(L);i++)
   {
      def Q=L[i];
      setring Q;
      ideal J=BO[2];
      list E=BO[4];
      map phi=R,BO[5];
      ideal K=phi(J)+BO[1];
      ideal stTK=std(K);

      if(loca)
      {
         ideal M=phi(M)+BO[1];
         ideal stTM=std(M);
      }
      for(j=1;j<=size(E);j++)
      {
         if(deg(E[j][1])>0)
         {
            stTK=sat(stTK,E[j])[1];
         }
         if(loca)
         {
            stTM=sat(stTM,E[j])[1];
         }
      }
      ideal sL=slocusE(J);
      if(loca){sL=sL+stTM;}
      ideal sLstd=std(sL);
      if(deg(sLstd[1])>0)
      {
         if(!loc)
         {
            "J is not smooth";i;
            setring R;
            return(0);
         }
         if(size(reduce(@p,std(radical(sLstd)),5))>0)
         {
            "J is not smooth";i;
            setring R;
            return(0);
         }
      }
      if(!((size(reduce(J,std(stTK),5))==0)
           &&(size(reduce(stTK,std(J),5))==0)))
      {
         "map is wrong";i;
         setring R;
         return(0);
      }
      if(loc){tr=transversalT(J,E,@p);}
      else{tr=transversalT(J,E);}
      if(!tr)
      {
         "E not transversal with J";i;
         setring R;
         return(0);
      }
      if(!normalCross(E))
      {
         "E not normal crossings";i;
         setring R;
         return(0);
      }
      for(j=1;j<=size(E);j++)
      {
         if(deg(E[j][1])>0){E[j]=E[j]+J;}
      }
      if(!normalCross(E))
      {
         "E not normal crossings with J";i;
         setring R;
         return(0);
      }
      kill J,E,phi,K,stTK;
      if(loca){kill M,stTM;}
      setring R;
      kill Q;
   }
   return(1);
}
//////////////////////////////////////////////////////////////////////////////
static proc testBlowUp(list BO,ideal cent,list tmpList, int j, int extra)
{
   def R=basering;
   int n=nvars(basering);
   int i;
   if((extra!=3)&&(extra!=2))
   {
      ideal K=BO[1],BO[2],cent;
      for(i=1;i<=size(BO[4]);i++)
      {
        K=K,BO[4][i];
      }
      list N=findvars(K);
      //list N=findvars(BO[2]);
      if(size(N[1])<n)
      {
         string newvar=string(N[1]);
         execute("ring R1=("+charstr(R)+"),("+newvar+"),dp;");
         list BO=imap(R,BO);
         ideal cent=imap(R,cent);
         n=nvars(R1);
      }
      else
      {
         def R1=basering;
      }
   }
   else
   {
      def R1=basering;
   }

   i=0;
   ideal T=cent;
   ideal TW;
   for(i=1;i<=size(tmpList);i++)
   {
      def Q=tmpList[i];
      setring Q;
      map phi=R1,lastMap;
      ideal TE=radical(slocusE(BO[2]));
      setring R1;
      TW=preimage(Q,phi,TE);
      T=intersect(T,TW);
      kill Q;
   }
   ideal sL=intersect(slocusE(BO[2]),cent);
   if(size(reduce(sL,std(radical(T)),5))>0){setring R;return(0);}
   if(size(reduce(T,std(radical(sL)),5))>0){setring R;return(0);}
   setring R;
   return(1);
}
//////////////////////////////////////////////////////////////////////////////
proc normalCross(list E,list #)
"Internal procedure - no help and no example available
"
{
   int loc;
   if((defined(locaT))&&(defined(@p)))
   {
      loc=1;
      ideal pp=@p;
   }
   int i,d,j;
   int n=nvars(basering);
   list E1,E2;
   ideal K,M,Estd,cent;
   intvec v,w;
   if(size(#)>0){cent=#[1];}

   for(i=1;i<=size(E);i++)
   {
      Estd=std(E[i]);
      if(deg(Estd[1])>0)
      {
         E1[size(E1)+1]=Estd;
      }
   }
   E=E1;
   for(i=1;i<=size(E);i++)
   {
      v=i;
      E1[i]=list(E[i],v);
   }
   list ll;
   int re=1;
   int ok;
   while(size(E1)>0)
   {
      K=E1[1][1];
      v=E1[1][2];
      attrib(K,"isSB",1);
      E1=delete(E1,1);
      d=n-dim(K);
      M=minor(jacob(K),d)+K;
      if(deg(std(M)[1])>0)
      {
         if(size(#)>0)
         {
            if(size(reduce(M,std(cent),5))>0)
            {
               ll[size(ll)+1]=std(M);
            }
            else
            {
               ok=1;
            }
         }
         if(!loc)
         {
            re=0;
         }
         else
         {
            if(size(reduce(pp,std(radical(M)),5))>0){re=0;}
         }
      }
      for(i=1;i<=size(E);i++)
      {
         for(j=1;j<=size(v);j++){if(v[j]==i){break;}}
         if(j<=size(v)){if(v[j]==i){i++;continue;}}
         Estd=std(K+E[i]);
         w=v;
         if(deg(Estd[1])==0){i++;continue;}
         if(d==n-dim(Estd))
         {
            if(size(#)>0)
            {
               if(size(reduce(Estd,std(cent),5))>0)
               {
                  ll[size(ll)+1]=Estd;
               }
               else
               {
                  ok=1;
               }
            }
            if(!loc)
            {
               re=0;
            }
            else
            {
               if(size(reduce(pp,std(radical(M)),5))>0){re=0;}
            }
         }
         w[size(w)+1]=i;
         E2[size(E2)+1]=list(Estd,w);
      }
      if(size(E2)>0)
      {
         if(size(E1)>0)
         {
            E1[size(E1)+1..size(E1)+size(E2)]=E2[1..size(E2)];
         }
         else
         {
            E1=E2;
         }
      }
      kill E2;
      list E2;
   }
/*
   if((!ok)&&(!re)&&(size(#)==1))
   {

      "the center is wrong";
      "it could be one of the following list";
      ll;
       ~;
   }
*/
   if((!ok)&&(!re)&&(size(#)==2))
   {
      return(2);   //for Center correction
   }
   return(re);
}
//////////////////////////////////////////////////////////////////////////////
static proc normalCrossB(ideal J,list E,ideal V)
"Internal procedure - no help and no example available
"
{
   int i,d,j;
   int n=nvars(basering);
   list E1,E2;
   ideal K,M,Estd;
   intvec v,w;

   for(i=1;i<=size(E);i++)
   {
      Estd=std(E[i]+J);
      if(deg(Estd[1])>0)
      {
         E1[size(E1)+1]=Estd;
      }
   }
   E=E1;
   for(i=1;i<=size(E);i++)
   {
      v=i;
      E1[i]=list(E[i],v);
   }
   list ll;
   int re=1;

   while((size(E1)>0)&&(re==1))
   {
      K=E1[1][1];
      v=E1[1][2];
      attrib(K,"isSB",1);
      E1=delete(E1,1);
      d=n-dim(K);
      M=minor(jacob(K),d)+K;
      if(deg(std(M+V)[1])>0)
      {
         re=0;
         break;
      }
      for(i=1;i<=size(E);i++)
      {
         for(j=1;j<=size(v);j++){if(v[j]==i){break;}}
         if(j<=size(v)){if(v[j]==i){i++;continue;}}
         Estd=std(K+E[i]);
         w=v;
         if(deg(Estd[1])==0){i++;continue;}
         if(d==n-dim(Estd))
         {
            if(deg(std(Estd+V)[1])>0)
            {
               re=0;
               break;
            }
         }
         w[size(w)+1]=i;
         E2[size(E2)+1]=list(Estd,w);
      }
      if(size(E2)>0)
      {
         if(size(E1)>0)
         {
            E1[size(E1)+1..size(E1)+size(E2)]=E2[1..size(E2)];
         }
         else
         {
            E1=E2;
         }
      }
      kill E2;
      list E2;
   }
   return(re);
}
//////////////////////////////////////////////////////////////////////////////
static proc norC(list BO,ideal cent)
"Internal procedure - no help and no example available
"
{
   int j;
   list  E=BO[4];
   ideal N=BO[2];
   if(BO[3][1]>1){return(0);}
   if(deg(std(slocusE(BO[2]))[1])>0){return(0);}
   if(!transversalT(N,E)){return(0);}
   for(j=1;j<=size(E);j++){if(deg(E[j][1])>0){E[j]=E[j]+N;}}
   if(!normalCross(E,cent)){return(0);}
   return(1);
}
//////////////////////////////////////////////////////////////////////////////
static proc specialReduce(ideal I,ideal J,poly p)
{
   matrix M;
   int i,j;
   for(i=1;i<=ncols(I);i++)
   {
      M=coeffs(I[i],@z);
      I[i]=0;
      for(j=1;j<=nrows(M);j++)
      {
         I[i]=I[i]+M[j,1]*p^(nrows(M)-j);
      }
      I[i]=reduce(I[i],J);
   }
   return(I);
}
