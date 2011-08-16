import unittest
from ost import *

class TestMMCifInfo(unittest.TestCase):
  def setUp(self):
    pass

  def test_mmcifinfo_citation(self):
    c = io.MMCifInfoCitation()
    # test ID setting/ getting
    c.SetID('ID')
    self.assertEquals(c.GetID(), 'ID')
    # test CAS setting/ getting
    c.SetCAS('FOO')
    self.assertEquals(c.GetCAS(), 'FOO')
    # test ISBN setting/ getting
    c.SetISBN('0-0-0-0-0-0')
    self.assertEquals(c.GetISBN(), '0-0-0-0-0-0')
    # test published_in setting/ getting
    c.SetPublishedIn('Best Book Ever')
    self.assertEquals(c.GetPublishedIn(), 'Best Book Ever')
    # test volume setting/ getting
    c.SetVolume('3')
    self.assertEquals(c.GetVolume(), '3')
    # test page setting/ getting
    c.SetPageFirst('1')
    self.assertEquals(c.GetPageFirst(), '1')
    c.SetPageLast('10')
    self.assertEquals(c.GetPageLast(), '10')
    # test doi setting/ getting
    c.SetDOI('HERE')
    self.assertEquals(c.GetDOI(), 'HERE')
    # test PubMed setting/ getting
    c.SetPubMed(815)
    self.assertEquals(c.GetPubMed(), 815)
    # test year setting/ getting
    c.SetYear(815)
    self.assertEquals(c.GetYear(), 815)
    # test title setting/ getting
    c.SetTitle('Foo')
    self.assertEquals(c.GetTitle(), 'Foo')
    # test auhtors setting/ getting
    s = ost.StringList()
    s.append('Foo')
    c.SetAuthorList(s)
    s2 = c.GetAuthorList()
    self.assertEquals(s2[0], 'Foo')

    i = io.MMCifInfo()
    i.SetMethod('Deep-Fry')
    i.SetResolution(2.0)
    i.AddCitation(c)
    s.append('Bar')
    i.AddAuthorsToCitation('ID', s)

    cl = i.GetCitations()
    self.assertEquals(len(cl), 1)
    al = cl[0].GetAuthorList()
    self.assertEquals(len(al), 2)
    self.assertEquals(al[0], 'Foo')
    self.assertEquals(al[1], 'Bar')

    self.assertEquals(i.GetMethod(), 'Deep-Fry')
    self.assertEquals(i.GetResolution(), 2.0)


  def test_mmcifinfo_biounit(self):
    b = io.MMCifInfoBioUnit()
    b.SetDetails('Details')
    self.assertEquals(b.GetDetails(), 'Details')
    b.AddChain('A')
    cl = b.GetChainList()
    self.assertEquals(cl[0], 'A')

    i = io.MMCifInfo()
    i.AddBioUnit(b)

    bl = i.GetBioUnits()
    self.assertEquals(len(bl), 1)


  def test_mmcifinfo_transoperation(self):
    o = io.MMCifInfoTransOp()
    o.SetID("1")
    self.assertEquals(o.GetID(), '1')
    o.SetType("identity operation")
    self.assertEquals(o.GetType(), 'identity operation')
    o.SetVector(1.0, 2.0, 3.0)
    self.assertEquals(o.GetVector().x, 1.0)
    self.assertEquals(o.GetVector().y, 2.0)
    self.assertEquals(o.GetVector().z, 3.0)
    o.SetMatrix(1, 2, 3, 4, 5, 6, 7, 8, 9)
    self.assertEquals(geom.Equal(o.GetMatrix(),
                                 geom.Mat3(1, 2, 3, 4, 5, 6, 7, 8, 9)), True)

    i = io.MMCifInfo()
    i.AddOperation(o)
    ol = i.GetOperations()
    self.assertEquals(ol[0].GetID(), '1')

    b = io.MMCifInfoBioUnit()
    b.AddOperations(ol)
    oll = b.GetOperations()
    self.assertEquals(oll[0][0].GetID(), '1')

if __name__== '__main__':
    unittest.main()


