/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include <assert.h>
#include "test.h"
#include <stdio.h>

#include "CLucene/search/MultiPhraseQuery.h"

	SimpleAnalyzer a;
	StandardAnalyzer aStd;
	WhitespaceAnalyzer aWS;
	IndexSearcher* s=NULL;

	void _TestSearchesRun(CuTest *tc, Analyzer* analyzer, Searcher* search, const TCHAR* qry){
		Query* q = NULL;
		Hits* h = NULL;
		try{
			q = QueryParser::parse(qry , _T("contents"), analyzer);
			if ( q != NULL ){
			    h = search->search( q );

			    if ( h->length() > 0 ){
			    //check for explanation memory leaks...
          CL_NS(search)::Explanation expl1;
					search->explain(q, h->id(0), &expl1);
					TCHAR* tmp = expl1.toString();
					_CLDELETE_CARRAY(tmp);
					if ( h->length() > 1 ){ //do a second one just in case
						CL_NS(search)::Explanation expl2;
						search->explain(q, h->id(1), &expl2);
						tmp = expl2.toString();
						_CLDELETE_CARRAY(tmp);
					}
				}
			}
    }catch(CLuceneError& err){
      CuFail(tc,_T("Error: %s\n"), err.twhat());
    }catch(...){
      CuFail(tc,_T("Error: unknown\n"));
    }
		_CLDELETE(h);
		_CLDELETE(q);
	}

	void testSrchOpenIndex(CuTest *tc ){
		char loc[1024];
		strcpy(loc, clucene_data_location);
		strcat(loc, "/reuters-21578-index");

		CuAssert(tc,_T("Index does not exist"), Misc::dir_Exists(loc));
		s=_CLNEW IndexSearcher(loc);
  }
	void testSrchCloseIndex(CuTest* /*tc*/ ){
		if ( s!=NULL ){
			s->close();
			_CLDELETE(s);
		}
  }

	void testSrchPunctuation(CuTest *tc ){
		CuAssert(tc,_T("Searcher was not open"),s!=NULL);

		//test punctuation
		_TestSearchesRun(tc, &a,s, _T("a&b") );
		_TestSearchesRun(tc, &a,s, _T("a&&b") );
		_TestSearchesRun(tc, &a,s, _T(".NET") );
	}

	void testSrchSlop(CuTest *tc ){
#ifdef NO_FUZZY_QUERY
		CuNotImpl(tc,_T("Fuzzy"));
#else
		CuAssert(tc,_T("Searcher was not open"),s!=NULL);
		//test slop
		_TestSearchesRun(tc, &a,s, _T("\"term germ\"~2") );
		_TestSearchesRun(tc, &a,s, _T("\"term germ\"~2 flork") );
		_TestSearchesRun(tc, &a,s, _T("\"term\"~2") );
		_TestSearchesRun(tc, &a,s, _T("\" \"~2 germ") );
		_TestSearchesRun(tc, &a,s, _T("\"term germ\"~2^2") );
#endif
	}

	void testSrchNumbers(CuTest *tc ){
		CuAssert(tc,_T("Searcher was not open"),s!=NULL);

		// The numbers go away because SimpleAnalzyer ignores them
		_TestSearchesRun(tc, &a,s, _T("3") );
		_TestSearchesRun(tc, &a,s, _T("term 1.0 1 2") );
		_TestSearchesRun(tc, &a,s, _T("term term1 term2") );

		_TestSearchesRun(tc, &aStd,s, _T("3") );
		_TestSearchesRun(tc, &aStd,s, _T("term 1.0 1 2") );
		_TestSearchesRun(tc, &aStd,s, _T("term term1 term2") );
	}

	void testSrchWildcard(CuTest *tc ){
#ifdef NO_WILDCARD_QUERY
		CuNotImpl(tc,_T("Wildcard"));
#else
		CuAssert(tc,_T("Searcher was not open"),s!=NULL);
		//testWildcard
		_TestSearchesRun(tc, &a,s, _T("term*") );
		_TestSearchesRun(tc, &a,s, _T("term*^2") );
		_TestSearchesRun(tc, &a,s, _T("term~") );
		_TestSearchesRun(tc, &a,s, _T("term^2~") );
		_TestSearchesRun(tc, &a,s, _T("term~^2") );
		_TestSearchesRun(tc, &a,s, _T("term*germ") );
		_TestSearchesRun(tc, &a,s, _T("term*germ^3") );

		//test problem reported by Gary Mangum
		BooleanQuery* bq = _CLNEW BooleanQuery();
		Term* upper = _CLNEW Term(_T("contents"),_T("0105"));
		Term* lower = _CLNEW Term(_T("contents"),_T("0105"));
		RangeQuery* rq=_CLNEW RangeQuery(lower,upper,true);
		bq->add(rq,true,true,false);
		_CLDECDELETE(upper);
		_CLDECDELETE(lower);

		Term* prefix = _CLNEW Term(_T("contents"),_T("reuters21578"));
		PrefixQuery* pq = _CLNEW PrefixQuery(prefix);
		_CLDECDELETE(prefix);
		bq->add(pq,true,true,false);

		Hits* h = NULL;
		try{
			h = s->search( bq );
		}_CLFINALLY(
		_CLDELETE(h);
		_CLDELETE(bq);
		);
#endif
	}

	void testSrchEscapes(CuTest *tc ){
		CuAssert(tc,_T("Searcher was not open"),s!=NULL);
		//testEscaped
		_TestSearchesRun(tc, &aWS,s, _T("\\[brackets") );
		_TestSearchesRun(tc, &a,s, _T("\\[brackets") );
		_TestSearchesRun(tc, &aWS,s, _T("\\\\") );
		_TestSearchesRun(tc, &aWS,s, _T("\\+blah") );
		_TestSearchesRun(tc, &aWS,s, _T("\\(blah") );
	}

	void testSrchRange(CuTest *tc ){
#ifdef NO_RANGE_QUERY
		CuNotImpl(tc,_T("Range"));
#else
		CuAssert(tc,_T("Searcher was not open"),s!=NULL);
		//testRange
		_TestSearchesRun(tc, &a,s, _T("[ j m]") );
		_TestSearchesRun(tc, &a,s, _T("[ j m ]") );
		_TestSearchesRun(tc, &a,s, _T("{ j m}") );
		_TestSearchesRun(tc, &a,s, _T("{ j m }") );
		_TestSearchesRun(tc, &a,s, _T("{a TO b}") );
		_TestSearchesRun(tc, &a,s, _T("{ j m }^2.0") );
		_TestSearchesRun(tc, &a,s, _T("[ j m] OR bar") );
		_TestSearchesRun(tc, &a,s, _T("[ j m] AND bar") );
		_TestSearchesRun(tc, &a,s, _T("( bar blar { j m}) ") );
		_TestSearchesRun(tc, &a,s, _T("gack ( bar blar { j m}) ") );
#endif
	}

	void testSrchSimple(CuTest *tc ){
		CuAssert(tc,_T("Searcher was not open"),s!=NULL);
    	//simple tests
		_TestSearchesRun(tc, &a,s, _T("a AND b") );

		_TestSearchesRun(tc, &a,s, _T("term term term") );

#ifdef _UCS2
		TCHAR tmp1[100];
		lucene_utf8towcs(tmp1,"t\xc3\xbcrm term term",100);
		_TestSearchesRun(tc, &a,s, tmp1 );

		lucene_utf8towcs(tmp1,"\xc3\xbcmlaut",100);
		_TestSearchesRun(tc, &a,s, tmp1 );
#endif

		_TestSearchesRun(tc, &a,s, _T("(a AND b)") );
		_TestSearchesRun(tc, &a,s, _T("c OR (a AND b)") );
		_TestSearchesRun(tc, &a,s, _T("a AND NOT b") );
		_TestSearchesRun(tc, &a,s, _T("a AND -b") );
		_TestSearchesRun(tc, &a,s, _T("a AND !b") );
		_TestSearchesRun(tc, &a,s, _T("a && b") );
		_TestSearchesRun(tc, &a,s, _T("a && ! b") );

		_TestSearchesRun(tc, &a,s, _T("a OR b") );
		_TestSearchesRun(tc, &a,s, _T("a || b") );
		_TestSearchesRun(tc, &a,s, _T("a OR !b") );
		_TestSearchesRun(tc, &a,s, _T("a OR ! b") );
		_TestSearchesRun(tc, &a,s, _T("a OR -b") );

		_TestSearchesRun(tc, &a,s, _T("+term -term term") );
		_TestSearchesRun(tc, &a,s, _T("foo:term AND field:anotherTerm") );
		_TestSearchesRun(tc, &a,s, _T("term AND \"phrase phrase\"") );
		_TestSearchesRun(tc, &a,s, _T("search AND \"meaningful direction\"") );
		_TestSearchesRun(tc, &a,s, _T("\"hello there\"") );

		_TestSearchesRun(tc, &a,s,  _T("a AND b") );
		_TestSearchesRun(tc, &a,s,  _T("hello") );
		_TestSearchesRun(tc, &a,s,  _T("\"hello there\"") );

		_TestSearchesRun(tc, &a,s, _T("germ term^2.0") );
		_TestSearchesRun(tc, &a,s, _T("term^2.0") );
		_TestSearchesRun(tc, &a,s, _T("term^2") );
		_TestSearchesRun(tc, &a,s, _T("term^2.3") );
		_TestSearchesRun(tc, &a,s, _T("\"germ term\"^2.0") );
		_TestSearchesRun(tc, &a,s, _T("\"term germ\"^2") );

		_TestSearchesRun(tc, &a,s, _T("(foo OR bar) AND (baz OR boo)") );
		_TestSearchesRun(tc, &a,s, _T("((a OR b) AND NOT c) OR d") );
		_TestSearchesRun(tc, &a,s, _T("+(apple \"steve jobs\") -(foo bar baz)") );

		_TestSearchesRun(tc, &a,s, _T("+title:(dog OR cat) -author:\"bob dole\"") );


		_TestSearchesRun(tc, &a,s, _T(".*") );
		_TestSearchesRun(tc, &a,s, _T("<*") );
		_TestSearchesRun(tc, &a,s, _T("/*") );
		_TestSearchesRun(tc, &a,s, _T(";*") );
	}

void SearchTest(CuTest *tc, bool bram) {
	uint64_t start = Misc::currentTimeMillis();

	SimpleAnalyzer analyzer;

	char fsdir[CL_MAX_PATH];
	_snprintf(fsdir,CL_MAX_PATH,"%s/%s",cl_tempDir, "test.search");
	Directory* ram = (bram?(Directory*)_CLNEW RAMDirectory():(Directory*)FSDirectory::getDirectory(fsdir) );

	IndexWriter writer( ram, &analyzer, true);
	writer.setUseCompoundFile(false);
  writer.setMaxBufferedDocs(3);

	const TCHAR* docs[] = { _T("a b c d e asdf"),
		_T("a b c d e a b c d e asdg"),
		_T("a b c d e f g h i j"),
		_T("a c e"),
		_T("e c a"),
		_T("a c e a c e asef"),
		_T("a c e a b c")
	};

	for (int j = 0; j < 7; j++) {
		Document* d = _CLNEW Document();
		//no need to delete fields... document takes ownership
		d->add(*_CLNEW Field(_T("contents"),docs[j],Field::STORE_YES | Field::INDEX_TOKENIZED));

		writer.addDocument(d);
		_CLDELETE(d);
	}
	writer.close();

	if (!bram){
		ram->close();
		_CLDECDELETE(ram);
		ram = (Directory*)FSDirectory::getDirectory(fsdir);
	}

	IndexReader* reader = IndexReader::open(ram);
	IndexSearcher searcher(reader);

	const TCHAR* queries[] = {
    _T("a AND NOT b"),
    _T("+a -b"),
		_T("\"a b\""),
		_T("\"a b c\""),
		_T("a AND b"),
		_T("a c"),
		_T("\"a c\""),
		_T("\"a c e\"")
	};
	int shouldbe[] = {3,3,4,4,4,7,3,3};
	Hits* hits = NULL;
	QueryParser parser(_T("contents"), &analyzer);

	for (int k = 0; k < 8; k++) {
		Query* query = parser.parse(queries[k]);

		TCHAR* qryInfo = query->toString(_T("contents"));
		hits = searcher.search(query);
		CLUCENE_ASSERT( hits->length() == shouldbe[k] );
		_CLDELETE_CARRAY(qryInfo);
		_CLDELETE(hits);
		_CLDELETE(query);
	}

  //test MultiPositionQuery...
  {
    MultiPhraseQuery* query = _CLNEW MultiPhraseQuery();
    RefCountArray<Term*> terms(3);
    Term* termE = _CLNEW Term(_T("contents"), _T("e"));
    terms[0] = _CLNEW Term(_T("contents"), _T("asdf"));
    terms[1] = _CLNEW Term(_T("contents"), _T("asdg"));
    terms[2] = _CLNEW Term(_T("contents"), _T("asef"));

    query->add(termE);
		_CLDECDELETE(termE);
    
    query->add(&terms);
    terms.deleteValues();

		TCHAR* qryInfo = query->toString(_T("contents"));
		hits = searcher.search(query);
		CLUCENE_ASSERT( hits->length() == 3 );
		_CLDELETE_CARRAY(qryInfo);
		_CLDELETE(hits);
		_CLDELETE(query);
  }
  
	searcher.close();
    reader->close();
	_CLDELETE( reader );

	ram->close();
	_CLDECDELETE(ram);

	CuMessageA (tc,"took %d milliseconds\n", (int32_t)(Misc::currentTimeMillis()-start));
}

void testNormEncoding(CuTest *tc) {
	//just a quick test of the default similarity
	CLUCENE_ASSERT(CL_NS(search)::Similarity::getDefault()->queryNorm(1)==1.0);

    float_t f = CL_NS(search)::Similarity::getDefault()->queryNorm(9);
    f -= (1.0/3.0);
    if ( f < 0 )
        f *= -1;
	CLUCENE_ASSERT(f < 0.1);

    //test that div by zero is handled
    float_t tmp = CL_NS(search)::Similarity::getDefault()->lengthNorm(_T("test"),0);
    tmp = CL_NS(search)::Similarity::getDefault()->queryNorm(0);

	//test that norm encoding is working properly
	CLUCENE_ASSERT( CL_NS(search)::Similarity::encodeNorm(-1)==0 );
	CLUCENE_ASSERT( CL_NS(search)::Similarity::encodeNorm(0)==0 );
	CLUCENE_ASSERT( CL_NS(search)::Similarity::encodeNorm(1)==124 );
	CLUCENE_ASSERT( CL_NS(search)::Similarity::encodeNorm(1)==124 );
	CLUCENE_ASSERT( CL_NS(search)::Similarity::encodeNorm(7516192768.0 )==255);


	CLUCENE_ASSERT( CL_NS(search)::Similarity::decodeNorm(124)==1 );
	CLUCENE_ASSERT( CL_NS(search)::Similarity::decodeNorm(255)==7516192768.0 );

    //well know value:
    CLUCENE_ASSERT( CL_NS(search)::Similarity::encodeNorm(0.5f) == 120 );

    //can decode self
    CLUCENE_ASSERT( CL_NS(search)::Similarity::encodeNorm(CL_NS(search)::Similarity::decodeNorm(57)) == 57 );
}

void testSrchManyHits(CuTest* /*tc*/) {
  SimpleAnalyzer analyzer;
	RAMDirectory ram;
	IndexWriter writer( &ram, &analyzer, true);

	const TCHAR* docs[] = { _T("a b c d e"),
		_T("a b c d e a b c d e"),
		_T("a b c d e f g h i j"),
		_T("a c e"),
		_T("e c a"),
		_T("a c e a c e"),
		_T("a c e a b c")
	};

	for (int j = 0; j < 140; j++) {
		Document* d = _CLNEW Document();
		//no need to delete fields... document takes ownership
		int x = j%7;
		d->add(*_CLNEW Field(_T("contents"),docs[x],Field::STORE_YES | Field::INDEX_TOKENIZED));

		writer.addDocument(d);
		_CLDELETE(d);
	}
	writer.close();

	IndexSearcher searcher(&ram);

	BooleanQuery query;
	Term* t = _CLNEW Term(_T("contents"), _T("a"));
	query.add(_CLNEW TermQuery(t),true,false, false);
	_CLDECDELETE(t);
	Hits* hits = searcher.search(&query);
	for ( size_t x=0;x<hits->length();x++ ){
	      hits->doc(x);
	}
	_CLDELETE(hits);
	searcher.close();
}

void testSrchMulti(CuTest *tc) {
  SimpleAnalyzer analyzer;
	RAMDirectory ram0;
	IndexWriter writer0( &ram0, &analyzer, true);

	const TCHAR* docs0[] = {
		_T("a")
	};

	Document* d = _CLNEW Document();
	//no need to delete fields... document takes ownership
	d->add(*_CLNEW Field(_T("contents"),docs0[0],Field::STORE_YES | Field::INDEX_TOKENIZED));

	writer0.addDocument(d);
	_CLDELETE(d);
	writer0.close();

	RAMDirectory ram1;
	IndexWriter writer1( &ram1, &analyzer, true);

	const TCHAR* docs1[] = {
		_T("e")
	};

	d = _CLNEW Document();
	//no need to delete fields... document takes ownership
	d->add(*_CLNEW Field(_T("contents"),docs1[0],Field::STORE_YES | Field::INDEX_TOKENIZED));

	writer1.addDocument(d);
	_CLDELETE(d);
	writer1.close();

	IndexSearcher searcher0(&ram0);
	IndexSearcher searcher1(&ram1);

	Searchable* searchers[3];

	searchers[0] = &searcher0;
	searchers[1] = &searcher1;
	searchers[2] = NULL;

	MultiSearcher searcher(searchers);

  Term* termA = _CLNEW Term(_T("contents"), _T("a"));
  Term* termC = _CLNEW Term(_T("contents"), _T("c"));
	RangeQuery query(termA, termC, true);
  _CLDECDELETE(termA);
  _CLDECDELETE(termC);

	Query* rewritten = searcher.rewrite(&query);
	Hits* hits = searcher.search(rewritten);
	for ( size_t x=0;x<hits->length();x++ ){
	  hits->doc(x);
	}
  CLUCENE_ASSERT(hits->length() == 1);
	if (&query != rewritten) {
		_CLDELETE(rewritten);
	}
	_CLDELETE(hits);
	searcher.close();
}

void ramSearchTest(CuTest *tc) { SearchTest(tc, true); }
void fsSearchTest(CuTest *tc) { SearchTest(tc, false); }



/////////////////////////////////////////////////////////////////////////////
Directory* prepareRAMDirectory( int nCount )
{
	const TCHAR*    tszDocText =  _T("KOL�N__C kol�n kol�n_cs NAD__C R�NEM__C r�nem r�n_cs �TA__C �ta �ta_cs V�sledek__C v�sledek v�sledek_cs dne�n�ho dne�n_cs utk�n� utk�n_cs skupiny skup_cs 2 2_cs Poh�ru__C poh�ru poh�r_cs mistr� mistr_cs evropsk�ch evropsk_cs zem� zem_cs ledn�m ledn_cs hokeji hokej_cs VEU__C veu veu_cs Feldkirch__C feldkirch feldkirch_cs Kol�n__C kol�n kol�n_cs 2 2_cs 4 4_cs 0 0_cs 1 1_cs 1 1_cs 1 1_cs 1 1_cs 2 2_cs Dom�c�__C dom�c� dom�c_cs sobotu sobot_cs utkaj� utkaj_cs fin�le fin�l_cs PMEz__C pmez pmez_cs Jokeritem__C jokeritem jokerit_cs Feldkirch__C feldkirch feldkirch_cs J�rk�pingem__C j�rk�pingem j�rk�ping_cs popasuje popasuj_cs t�et� t�et_cs OLOMOUC__C olomouc olomouc_cs �TA__C �ta �ta_cs K__C vojensk�m vojensk_cs �tvar�m �tvar_cs 2 2_cs arm�dn�ho arm�dn_cs sboru sbor_cs s�dlem s�dl_cs Olomouci__C olomouci olomouc_cs dnes dnes_cs povol�vac�ch povol�vac_cs rozkaz� rozkaz_cs nastoupit nastoupit_cs celkem celk_cs 1639 1639_cs nov��k� nov��k_cs v�konu v�kon_cs z�kladn� z�kladn_cs vojensk� vojensk_cs slu�by slu�b_cs Jejich__C 95 95_cs koleg� koleg_cs svou svo_cs slu�bu slu�b_cs sboru sbor_cs nasluhuje nasluhuj_cs p�i�em� p�i�em�_cs 1226 1226_cs voj�k� voj�k_cs z�kladn� z�kladn_cs slu�by slu�b_cs ode�lo ode�l_cs civilu civil_cs 22 22_cs prosince prosinc_cs lo�sk�ho lo�sk_cs roku rok_cs �TA__C �ta �ta_cs informoval informoval_cs n��eln�k n��eln�k_cs skupiny skup_cs styk styk_cs ve�ejnost� ve�ejnost_cs 2 2_cs arm�dn�ho arm�dn_cs sboru sbor_cs Olomouci__C olomouci olomouc_cs Dalibor__C dalibor dalibor_cs H�ib__C h�ib h�ib_cs Michal__C michal michal_cs �verd�k__C �verd�k �verd�k_cs �ENEVA__C �eneva �enev_cs �TA__C �ta �ta_cs OSN__C osn osn_cs uzav�e uzav�_cs nejpozd�ji nejpozd�j_cs konce konc_cs t�dne t�dn_cs leteck� leteck_cs most most_cs Sarajeva__C sarajeva sarajev_cs Ozn�mil__C ozn�mil ozn�mil_cs dnes dnes_cs �enev�__C �enev� �enev_cs mluv�� mluv�_cs Vysok�ho__C vysok�ho vysok_cs komisa�e komisa�_cs OSN__C osn osn_cs uprchl�ky uprchl�k_cs UNHCR__C unhcr unhcr_cs Ron__C ron ron_cs Redmond__C redmond redmond_cs Leteck�__C leteck� leteck_cs most most_cs roku rok_cs 1992 1992_cs obs�hl obs�hl_cs 13 13_cs 000 000_cs let� let_cs p�epravil p�epravil_cs 160 160_cs 000 000_cs tun tun_cs potravin potrav_cs prost�edk� prost�edk_cs humanit�rn� humanit�rn_cs pomoci pomoc_cs dlouhou dlouh_cs dobu dob_cs jedinou jed_cs z�sobovac� z�sobovac_cs cestou cest_cs Sarajeva__C sarajeva sarajev_cs oble�en�ho oble�en_cs bosensk�mi bosensk_cs Srby__C srby srb_cs Posledn�ch__C posledn�ch posledn_cs 210 210_cs tun tun_cs materi�lu materi�l_cs le�� le�_cs skladech sklad_cs italsk�m italsk_cs p��stavu p��stav_cs Ancona__C ancona ancon_cs dopraveny dopraven_cs ur�en� ur�en_cs n�kolika n�kolik_cs p��t�ch p��t_cs dnech dnech_cs Podle__C �daj� �daj_cs OSN__C osn osn_cs zem� zem_cs b�val� b�val_cs Jugosl�vie__C jugosl�vie jugosl�vi_cs roku rok_cs 1991 1991_cs dod�no dod�n_cs 1 1_cs 100 100_cs 000 000_cs tun tun_cs humanit�rn� humanit�rn_cs pomoci pomoc_cs Zden�k__C zden�k zden�k_cs Altynski__C altynski altynsk_cs KOL�N__C kol�n kol�n_cs NAD__C R�NEM__C r�nem r�n_cs �TA__C �ta �ta_cs V�sledek__C v�sledek v�sledek_cs dne�n�ho dne�n_cs utk�n� utk�n_cs skupiny skup_cs 2 2_cs Poh�ru__C poh�ru poh�r_cs mistr� mistr_cs evropsk�ch evropsk_cs zem� zem_cs ledn�m ledn_cs hokeji hokej_cs �ta �ta_cs cs cs_cs 4 4_cs 000 000_cs cs cs_cs 3 3_cs olomouc olomouc_cs cs cs_cs 3 3_cs osn osn_cs cs cs_cs 3 3_cs rok rok_cs cs cs_cs 3 3_cs sbor sbor_cs cs cs_cs 3 3_cs slu�b slu�b_cs cs cs_cs 3 3_cs tun tun_cs cs cs_cs 3 3_cs arm�dn arm�dn_cs cs cs_cs 2 2_cs dnes dnes_cs cs cs_cs 2 2_cs feldkirch feldkirch_cs cs cs_cs 2 2_cs humanit�rn humanit�rn_cs cs cs_cs 2 2_cs kol�n kol�n_cs cs cs_cs 2 2_cs leteck leteck_cs cs cs_cs 2 2_cs most most_cs cs cs_cs 2 2_cs pomoc pomoc_cs cs cs_cs 2 2_cs sarajev sarajev_cs cs cs_cs 2 2_cs skup skup_cs cs cs_cs 2 2_cs vojensk vojensk_cs cs cs_cs 2 2_cs zem zem_cs cs cs_cs 2 2_cs z�kladn z�kladn_cs cs cs_cs 2 2_cs �enev �enev_cs cs cs_cs 2 2_cs 100 100_cs cs cs_cs 1 1_cs 1226 1226_cs cs cs_cs 1 1_cs 160 160_cs cs cs_cs 1 1_cs 1639 1639_cs cs cs_cs 1 1_cs 1991 1991_cs cs cs_cs 1 1_cs 1992 1992_cs cs cs_cs 1 1_cs 210 210_cs cs cs_cs 1 1_cs altynsk altynsk_cs cs cs_cs 1 1_cs ancon ancon_cs cs cs_cs 1 1_cs bosensk bosensk_cs cs cs_cs 1 1_cs" );
    
	char fsdir[CL_MAX_PATH];
	_snprintf(fsdir,CL_MAX_PATH,"%s/%s",cl_tempDir, "test.search");
    Directory*      pDirectory = (Directory*)FSDirectory::getDirectory(fsdir);
    WhitespaceAnalyzer analyzer;
	IndexWriter     writer( pDirectory, &analyzer, true );
    writer.setUseCompoundFile( false );
	
    Document* d = _CLNEW Document();

    d->add( *_CLNEW Field( _T("_content"), tszDocText, Field::STORE_NO | Field::INDEX_TOKENIZED ));
    d->add( *_CLNEW Field( _T("_docid"), _T("0000000001"),   Field::STORE_YES | Field::INDEX_UNTOKENIZED ));
    d->add( *_CLNEW Field( _T("_gwkey"), _T("doc/doc.txt"),   Field::STORE_YES | Field::INDEX_UNTOKENIZED ));
    d->add( *_CLNEW Field( _T("_gwname"), _T("file://"),   Field::STORE_YES | Field::INDEX_UNTOKENIZED ));
    d->add( *_CLNEW Field( _T("_numkey"), _T("0000000001"),   Field::STORE_YES | Field::INDEX_UNTOKENIZED ));
    d->add( *_CLNEW Field( _T("_seccache"), _T("AAECAAAAAAAFIAAAACACAAAAAQEAAAAAAAUSAAAAAAEBAAAAAAAFCwAAAAABAgAAAAAABSAAAAAhAgAABA=="), Field::STORE_YES | Field::INDEX_NO ));
    d->add( *_CLNEW Field( _T("_secobject"), _T("doc/doc.txt"),   Field::STORE_YES | Field::INDEX_NO ));
    d->add( *_CLNEW Field( _T("_security"), _T("0000000033"),   Field::STORE_YES | Field::INDEX_NO ));
    d->add( *_CLNEW Field( _T("_termvector"), _T("�ta_cs_4 �ta_cs_4_cs, 000 000_cs, _cs_3 _cs_3_cs, abcd_cs_3 abcd_cs_3_cs, olomouc_cs_3 olomouc_cs_3_cs, rok_cs_3 rok_cs_3_cs, sbor_cs_3 sbor_cs_3_cs, slu�b_cs_3 slu�b_cs_3_cs, tun_cs_3 tun_cs_3_cs, arm�dn_cs_2 arm�dn_cs_2_cs, dnes_cs_2 dnes_cs_2_cs, feldkirch_cs_2 feldkirch_cs_2_cs, humanit�rn_cs_2 humanit�rn_cs_2_cs, kal_cs_2 kal_cs_2_cs, leteck_cs_2 leteck_cs_2_cs, most_cs_2 most_cs_2_cs, pomoc_cs_2 pomoc_cs_2_cs, sarajev_cs_2 sarajev_cs_2_cs, skup_cs_2 skup_cs_2_cs, vojensk_cs_2 vojensk_cs_2_cs, zem_cs_2 zem_cs_2_cs, z�kladn_cs_2 z�kladn_cs_2_cs, �enev_cs_2 �enev_cs_2_cs, 100 100_cs, _cs_1 _cs_1_cs, 1226 1226_cs, _cs_1 _cs_1_cs, 160 160_cs, _cs_1 _cs_1_cs, 1639 1639_cs, _cs_1 _cs_1_cs, 1991 1991_cs, _cs_1 _cs_1_cs, 1992 1992_cs, _cs_1 _cs_1_cs, 210 210_cs, _cs_1 _cs_1_cs, altynsk_cs_1 altynsk_cs_1_cs, ancon_cs_1 ancon_cs_1_cs, bosensk_cs_1 bosensk_cs_1_cs"),   Field::STORE_YES | Field::INDEX_TOKENIZED ));
    d->add( *_CLNEW Field( _T("_timestamp"), _T("2010-12-10 14:01:23.244493"),   Field::STORE_YES | Field::INDEX_NO ));
    d->add( *_CLNEW Field( _T("date"), _T("2010-12-10 14:01:08"),   Field::STORE_YES | Field::INDEX_UNTOKENIZED ));
    d->add( *_CLNEW Field( _T("key"), _T("doc/doc.txt"),   Field::STORE_YES | Field::INDEX_UNTOKENIZED ));
    d->add( *_CLNEW Field( _T("lang"), _T("cs"),   Field::STORE_YES | Field::INDEX_UNTOKENIZED ));
    d->add( *_CLNEW Field( _T("size"), _T("0000001555"),   Field::STORE_YES | Field::INDEX_UNTOKENIZED ));
    d->add( *_CLNEW Field( _T("summary"), _T("KOL�N__C kol�n kol�n_cs NAD__C R�NEM__C r�n_cs r�nem �TA__C �ta �ta_cs V�sledek__C v�sledek v�sledek_cs dne�n_cs dne�n�ho utk�n_cs utk�n� skup_cs skupiny 2 2_cs Poh�ru__C poh�r_cs poh�ru mistr_cs mistr� evropsk_cs evropsk�ch zem_cs zem� ledn_cs ledn�m hokej_cs hokeji VEU__C veu veu_cs Feldkirch__C feldkirch feldkirch_cs Kal__C kal kal_cs 2 2_cs 4 4_cs 0 0_cs 1 1_cs 1 1_cs 1 1_cs 1 1_cs 2 2_cs Dom�c�__C dom�c_cs dom�c� sobot_cs sobotu utkaj_cs utkaj� fin�l_cs fin�le PMEz__C pmez pmez_cs Jokeritem__C jokerit_cs jokeritem Feldkirch__C feldkirch feldkirch_cs J�rk�pingem__C j�rk�ping_cs j�rk�pingem popasuj_cs popasuje t�et_cs t�et�"),   Field::STORE_YES | Field::INDEX_TOKENIZED ));
    
    
    writer.addDocument(d);
	_CLDELETE( d );
    writer.close();
    return pDirectory;
}


/////////////////////////////////////////////////////////////////////////////
int createWord( int nWord, TCHAR* rgBuffer )
{
    rgBuffer[ 0 ] = _T( '\0' );
    
    int nMod = _T('z') - _T('a') + 1;
    int nPos = 0;
    do 
    {
        rgBuffer[ nPos ] = _T( 'a' ) + ( nWord % nMod );
        nWord = nWord / nMod;
        nPos++;
    } 
    while ( nWord > 0 );

    rgBuffer[ nPos ] = _T( '\0' );
    return nPos;
}

/////////////////////////////////////////////////////////////////////////////
Directory* prepareDirectory( int nCount )
{
	TCHAR    tszDocText[1024*1024];
    TCHAR *  tszWrite = tszDocText;    
	
    char fsdir[CL_MAX_PATH];
	_snprintf(fsdir,CL_MAX_PATH,"%s/%s",cl_tempDir, "test.search");
    Directory*      pDirectory = (Directory*)FSDirectory::getDirectory(fsdir);
    WhitespaceAnalyzer analyzer;
	IndexWriter     writer( pDirectory, &analyzer, true );
    writer.setUseCompoundFile( true );
	
    Document* d = _CLNEW Document();

    for( int nWord = 0; nWord <= nCount; nWord++ )
    {
        tszWrite += createWord( nWord, tszWrite );
        *tszWrite = _T( ' ' );
        tszWrite++;
    }
    *tszWrite = _T( '\0' );

    d->add( *_CLNEW Field( _T("_content"), tszDocText, Field::STORE_NO | Field::INDEX_TOKENIZED ));

    writer.addDocument(d);
	_CLDELETE( d );
    writer.close();

    return pDirectory;
}

/////////////////////////////////////////////////////////////////////////////
void runTest(CuTest *tc, int nSize)
{
   //Directory*      pDirectory = FSDirectory::getDirectory( "d:/ProgramData/Colls/case/index/index" );
    Directory*      pDirectory = prepareDirectory( nSize );
    Analyzer *      pAnalyzer   = NULL;
    Hits *          pHits       = NULL;
    IndexReader*    pReader = IndexReader::open( pDirectory );
    IndexSearcher   searcher( pReader );
    TCHAR           rgBuffer[128];

    CLUCENE_ASSERT( pReader->numDocs() == 1 );
    
    for( int nWord = 0; nWord <= nSize; nWord++ )
    {
        createWord( nWord, rgBuffer );

        // printf( "%d - %ls", nWord, rgBuffer );

        Term * t1 = new Term( _T( "_content" ), rgBuffer );
        TermQuery * pQry1 = new TermQuery( t1 );
        _CLDECDELETE( t1 );

        pAnalyzer = new SimpleAnalyzer();
        pHits = searcher.search( pQry1 );
        _ASSERT( pHits->length() == 1 );
        CLUCENE_ASSERT( pHits->length() == 1 );
        _CLDELETE( pHits );

        _CLDELETE( pAnalyzer );
        pHits = searcher.search( pQry1 );
        _ASSERT( pHits->length() == 1 );
        CLUCENE_ASSERT( pHits->length() == 1 );
        _CLDELETE( pHits );

        _CLDELETE( pQry1 );
    }

    searcher.close();
    _CLDELETE( pReader );

    pDirectory->close();
    _CLDECDELETE( pDirectory );
}


/////////////////////////////////////////////////////////////////////////////
void testRepetitiveSearch(CuTest *tc)
{
    for( int nSize = 0; nSize < 1000; nSize++ )
    {
//        printf( "%d\n", nSize );
        runTest( tc, nSize );
    }
}


/////////////////////////////////////////////////////////////////////////////
Directory* prepareDirectory1()
{
	const TCHAR * tszDocText = _T( "a b c d e f g h i j k l m n o p q r s t u v w x y z ab bb cb db eb fb gb hb ib jb kb lb mb nb ob pb qb rb sb tb ub vb wb xb yb zb ac bc cc dc ec fc gc hc ic jc kc lc mc nc oc pc qc rc sc tc uc vc wc xc yc zc ad bd cd dd ed fd gd hd id jd kd ld md nd od pd qd rd sd td ud vd wd xd yd zd ae be ce de ee fe ge he ie je ke le me ne oe pe qe re se te ue ve we xe ye ze af bf cf df ef ff gf hf if jf kf lf mf" );
	
    char fsdir[CL_MAX_PATH];
	_snprintf(fsdir,CL_MAX_PATH,"%s/%s",cl_tempDir, "test.search");

    WhitespaceAnalyzer  analyzer;
    Directory*          pDirectory = (Directory*)FSDirectory::getDirectory(fsdir);
	IndexWriter         writer( pDirectory, &analyzer, true );

    writer.setUseCompoundFile( false );
	
    Document* d = _CLNEW Document();
    d->add( *_CLNEW Field( _T("_content"), tszDocText, Field::STORE_NO | Field::INDEX_TOKENIZED ));
    writer.addDocument(d);
	_CLDELETE( d );
    writer.close();

    return pDirectory;
}

void testReadPastEOF(CuTest *tc)
{
   //Directory*      pDirectory = FSDirectory::getDirectory( "d:/ProgramData/Colls/case/index/index" );
    Directory*      pDirectory = prepareDirectory1();
    Analyzer *      pAnalyzer   = NULL;
    Hits *          pHits       = NULL;
    IndexReader*    pReader = IndexReader::open( pDirectory );
    IndexSearcher   searcher( pReader );

    CLUCENE_ASSERT( pReader->numDocs() == 1 );

    Term * t1 = new Term( _T( "_content" ), _T( "ze" ) );
    TermQuery * pQry1 = new TermQuery( t1 );
    _CLDECDELETE( t1 );

    pAnalyzer = new SimpleAnalyzer();
    pHits = searcher.search( pQry1 );
    _ASSERT( pHits->length() == 1 );
    CLUCENE_ASSERT( pHits->length() == 1 );
    _CLDELETE( pHits );

    _CLDELETE( pAnalyzer );
    pHits = searcher.search( pQry1 );
    _ASSERT( pHits->length() == 1 );
    CLUCENE_ASSERT( pHits->length() == 1 );
    _CLDELETE( pHits );

    _CLDELETE( pQry1 );

    searcher.close();
    _CLDELETE( pReader );

    pDirectory->close();
    _CLDECDELETE( pDirectory );
}


/////////////////////////////////////////////////////////////////////////////
CuSuite *testsearch(void)
{
	CuSuite *suite = CuSuiteNew(_T("CLucene Search Test"));
    SUITE_ADD_TEST(suite, ramSearchTest);
	SUITE_ADD_TEST(suite, fsSearchTest);

	SUITE_ADD_TEST(suite, testNormEncoding);
	SUITE_ADD_TEST(suite, testSrchManyHits);
	SUITE_ADD_TEST(suite, testSrchMulti);
	SUITE_ADD_TEST(suite, testSrchOpenIndex);
	SUITE_ADD_TEST(suite, testSrchPunctuation);
	SUITE_ADD_TEST(suite, testSrchSlop);
	SUITE_ADD_TEST(suite, testSrchNumbers);
	SUITE_ADD_TEST(suite, testSrchWildcard);
	SUITE_ADD_TEST(suite, testSrchEscapes);
	SUITE_ADD_TEST(suite, testSrchRange);
	SUITE_ADD_TEST(suite, testSrchSimple);
	SUITE_ADD_TEST(suite, testSrchCloseIndex);
//    SUITE_ADD_TEST(suite, testRepetitiveSearch); 
    SUITE_ADD_TEST(suite, testReadPastEOF);

    return suite;
}
// EOF
