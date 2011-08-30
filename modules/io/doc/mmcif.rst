mmCIF File Format
--------------------------------------------------------------------------------

The mmCIF file format is an alternate container for structural entities, also
provided by the PDB. Here we describe how to load those files and how to deal
with information provided above the common PDB format.


Loading mmCIF Files
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. autofunction:: ost.io.LoadMMCIF


Categories Available
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following categories of a mmCIF file are considered by the parser:

* ``atom_site``: Used to build the :class:`entity <ost.mol.EntityHandle>`
* ``entity``: Involved in setting ChainTypes
* ``entity_poly``: Involved in setting ChainTypes
* ``citation``: Goes into :class:`MMCifInfoCitation`
* ``citation_author``: Goes into :class:`MMCifInfoCitation`
* ``exptl``: Goes into :class:`MMCifInfo` as :attr:`method <MMCifInfo.method>`.
* ``refine``: Goes into :class:`MMCifInfo` as
  :attr:`resolution <MMCifInfo.resolution>`.
* ``pdbx_struct_assembly``: Used for :class:`MMCifInfoBioUnit`.
* ``pdbx_struct_assembly_gen``: Used for :class:`MMCifInfoBioUnit`.
* ``pdbx_struct_oper_list``: Used for :class:`MMCifInfoBioUnit`.


Info Classes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Information from mmCIF files which goes beyond structural data, is kept in a
special container, the :class:`MMCifInfo` class. Here is a detailed description
of the annotation available.

.. class:: MMCifInfo

  This is the container for all bits of non-molecular data pulled from a mmCIF
  file.

  .. attribute:: citations

    Stores a list of citations (:class:`MMCifInfoCitation`).

    Also available as :meth:`GetCitations`.

  .. attribute:: biounits

    Stores a list of biounits (:class:`MMCifInfoBioUnit`).

    Also available as :meth:`GetBioUnits`.

  .. attribute:: method

    Stores the experimental method used to create the structure.

    Also available as :meth:`GetMethod`. May also be modified by
    :meth:`SetMethod`.

  .. attribute:: resolution

    Stores the resolution of the crystal structure.

    Also available as :meth:`GetResolution`. May also be modified by
    :meth:`SetResolution`.

  .. attribute:: operations

    Stores the operations needed to transform a crystal structure into a
    biounit.

    Also available as :meth:`GetOperations`. May also be modified by
    :meth:`AddOperation`.

  .. method:: AddCitation(citation)

    Add a citation to the citation list of an info object.

    :param citation: Citation to be added.
    :type citation: :class:`MMCifInfoCitation`

  .. method:: AddAuthorsToCitation(id, authors)

    Adds a list of authors to a specific citation.

    :param id: identifier of the citation
    :type id: :class:`str`
    :param authors: List of authors.
    :type authors: :class:`~ost.StringList`

  .. method:: GetCitations()

    See :attr:`citations`

  .. method:: AddBioUnit(biounit)

    Add a biounit to the biounit list of an info object.

    :param biounit: Biounit to be added.
    :type biounit: :class:`MMCifInfoBioUnit`

  .. method:: GetBioUnits()

    See :attr:`citations`

  .. method:: SetMethod(method)

    See :attr:`method`

  .. method:: GetMethod()

    See :attr:`method`

  .. method:: SetResolution(resolution)

    See :attr:`resolution`

  .. method:: GetResolution()

    See :attr:`resolution`

  .. method:: AddOperation(operation)

    See :attr:`operations`

  .. method:: GetOperations()

    See :attr:`operations`

.. class:: MMCifInfoCitation

  This stores citation information from an input file.

  .. attribute:: id

    Stores an internal identifier for a citation. If not provided, resembles an
    empty string.

    Also available as :meth:`GetID`. May also be modified by :meth:`SetID`.

  .. attribute:: cas

    Stores a Chemical Abstract Service identifier, if available. If not
    provided, resembles an empty string.

    Also available as :meth:`GetCAS`. May also be modified by :meth:`SetCas`.

  .. attribute:: isbn

    Stores the ISBN code, presumably for cited books.  If not
    provided, resembles an empty string.

    Also available as :meth:`GetISBN`. May also be modified by :meth:`SetISBN`.

  .. attribute:: published_in

    Stores the book or journal title of a publication. Should take the full
    title, no abbreviations. If not provided, resembles an empty string.

    Also available as :meth:`GetPublishedIn`. May also be modified by
    :meth:`SetPublishedIn`.

  .. attribute:: volume

    Supposed to store volume information for journals. Since the volume number
    is not always a simple integer, it is stored as a string. If not provided,
    resembles an empty string.

    Also available as :meth:`GetVolume`. May also be modified by
    :meth:`SetVolume`.

  .. attribute:: page_first

    Stores the first page of a publication. Since the page numbers are not
    always a simple integers, they are stored as strings. If not provided,
    resembles empty strings.

    Also available as :meth:`GetPageFirst`. May also be modified by
    :meth:`SetPageFirst`.

  .. attribute:: page_last

    Stores the last page of a publication. Since the page numbers are not
    always a simple integers, they are stored as strings. If not provided,
    resembles empty strings.

    Also available as :meth:`GetPageLast`. May also be modified by
    :meth:`SetPageLast`.

  .. attribute:: doi

    Stores the Document Object Identifier as used by doi.org for a cited
    document. If not provided, resembles a empty strings.

    Also available as :meth:`GetDOI`. May also be modified by :meth:`SetDOI`.

  .. attribute:: pubmed

    Stores the PubMed accession number. If not provided, is set to 0.

    Also available as :meth:`GetPubMed`. May also be modified by
    :meth:`SetPubmed`.

  .. attribute:: year

    Stores the publication year. If not provided, is set to 0.

    Also available as :meth:`GetYear`. May also be modified by :meth:`SetYear`.

  .. attribute:: title

    Stores a title. If not provided, is set to an empty string.

    Also available as :meth:`GetTitle`. May also be modified by
    :meth:`SetTitle`.

  .. attribute:: authors

    Stores a :class:`~ost.StringList` of authors.

    Also available as :meth:`GetAuthorList`. May also be modified by
    :meth:`SetAuthorList`.

  .. method:: GetCAS()
    
    See :attr:`cas`

  .. method:: SetCAS(cas)

    See :attr:`cas`

  .. method:: GetISBN()
    
    See :attr:`isbn`

  .. method:: SetISBN(isbn)

    See :attr:`isbn`

  .. method:: GetPublishedIn()
    
    See :attr:`published_in`

  .. method:: SetPublishedIn(title)

    See :attr:`published_in`

  .. method:: GetVolume()
    
    See :attr:`volume`

  .. method:: SetVolume(volume)

    See :attr:`volume`

  .. method:: GetPageFirst()
    
    See :attr:`page_first`

  .. method:: SetPageFirst(first)

    See :attr:`page_first`

  .. method:: GetPageLast()
    
    See :attr:`page_last`

  .. method:: SetPageLast(last)

    See :attr:`page_last`

  .. method:: GetDOI()
    
    See :attr:`doi`

  .. method:: SetDOI(doi)

    See :attr:`doi`

  .. method:: GetPubMed()
    
    See :attr:`pubmed`

  .. method:: SetPubMed(no)

    See :attr:`pubmed`

  .. method:: GetYear()
    
    See :attr:`year`

  .. method:: SetYear(year)

    See :attr:`year`

  .. method:: GetTitle()
    
    See :attr:`title`

  .. method:: SetTitle(title)

    See :attr:`title`

  .. method:: GetAuthorList()

    See :attr:`authors`

  .. method:: SetAuthorList(list)

    See :attr:`authors`


.. class:: MMCifInfoTransOperation

  This stores operations needed to transform an
  :class:`entity <ost.mol.EntityHandle>` into a biounit.

  .. attribute:: id

    A unique identifier. If not provided, resembles an empty string.

    Also available as :meth:`GetID`. May also be modified by
    :meth:`SetID`.

  .. attribute:: type

    Describes the operation. If not provided, resembles an empty string.

    Also available as :meth:`GetType`. May also be modified by
    :meth:`SetType`.

  .. attribute:: translation

    The translational vector. Also available as :meth:`GetVector`. May also be

    modified by :meth:`SetVector`.

  .. attribute:: rotation

    The rotational matrix. Also available as :meth:`GetMatrix`. May also be

    modified by :meth:`SetMatrix`.

  .. method:: GetID()

    See :attr:`id`

  .. method:: SetID(id)

    See :attr:`id`

  .. method:: GetType()

    See :attr:`type`

  .. method:: SetType(type)

    See :attr:`type`

  .. method:: GetVector()

    See :attr:`translation`

  .. method:: SetVector(x, y, z)

    See :attr:`translation`

  .. method:: GetMatrix()

    See :attr:`rotation`

  .. method:: SetMatrix(i00,i01, i02, i10,i11, i12, i20,i21, i22)

    See :attr:`rotation`

.. class:: MMCifInfoBioUnit

  This stores information how a structure is to be assembled to form the
  biounit.

  .. attribute:: details

    Special aspects of the biological assembly. If not provided, resembles an
    empty string.

    Also available as :meth:`GetDetails`. May also be modified by
    :meth:`SetDetails`.

  .. attribute:: chains

    Chains involved in this biounit. If not provided, resembles an empty list.

    Also available as :meth:`GetChainList`. May also be modified by
    :meth:`AddChain`.

  .. attribute:: operations

    Translations and rotations needed to create the biounit. Filled with
    objects of class :class:`MMCifInfoTransOperation`.

    Also available as :meth:`GetOperations`. May be modified by
    :meth:`AddOperations`

  .. method:: GetDetails()

    See :attr:`details`

  .. method:: SetDetails(details)

    See :attr:`details`

  .. method:: GetChainList()

    See :attr:`chains`

  .. method:: AddChain(chain name)

    See :attr:`chains`

  .. method:: GetOperations()

    See :attr:`operations`

  .. method:: AddOperations(list of operations)

    See :attr:`operations`