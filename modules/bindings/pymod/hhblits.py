'''HHblits wrapper.
'''

import subprocess
import datetime
import os
import shutil
import tempfile

import ost
from ost import settings, seq
from ost.bindings import utils

class HHblitsHit:
    """
    A hit found by HHblits

    .. attribute:: hit_id

      String identifying the hit

    .. attribute:: aln

      Pairwise alignment containing the aligned part between the query and the
      target. First sequence is the query, the second sequence the target.

      :type: :class:`ost.seq.AlignmentHandle`

    .. attribute:: evalue

      The E-value of the alignment

    .. attribute:: prob

      The probability of the alignment (between 0 and 100)

    .. attribute:: score

      The alignment score
    """
    def __init__(self, hit_id, aln, score, ss_score, evalue, pvalue, prob):
        self.hit_id = hit_id
        self.aln = aln
        self.score = score
        self.ss_score = ss_score
        self.evalue = evalue
        self.prob = prob
        self.pvalue = pvalue

class HHblitsHeader:
    def __init__(self):
        self.query = ''
        self.match_columns = 0
        self.n_eff = 0
        self.searched_hmms = 0
        self.date = None
        self.command = ''

def ParseHeaderLine(line):
    '''Fetch header content.

    First, we seek the start of the identifier, that is, the first whitespace
    after the hit number + 1. Since the identifier may contain whitespaces
    itself, we cannot split the whole line
    '''
    for i in range(0, len(line)):
        if line[i].isdigit():
            break
    for i in range(i, len(line)):
        if line[i] == ' ':
            break
    assert len(line)-i >= 31 and line[i+1] != ' '
    hit_id = line[i+1:i+31].strip()
    fields = line[i+32:].split()
    prob = float(fields[0])
    evalue = float(fields[1])
    pvalue = float(fields[2])
    score = float(fields[3])
    ss_score = float(fields[4])
    offsets = (int(fields[6].split('-')[0]), int(fields[7].split('-')[0]))
    return (HHblitsHit(hit_id, None, score, ss_score, evalue, pvalue, prob),
            offsets)

def ParseHHblitsOutput(output):
    """
    Parses the HHblits output and returns a tuple of HHblitsHeader and
    a list of HHblitsHit instances.
    """
    lines = iter(output)
    def _ParseHeaderSection(lines):
        value_start_column = 14
        date_pattern = '%a %b %d %H:%M:%S %Y'
        header = HHblitsHeader()
        line = lines.next()
        assert line.startswith('Query')
        header.query = line[value_start_column:].strip()
        line = lines.next()
        assert line.startswith('Match_columns')
        header.match_columns = int(line[value_start_column:].strip())

        line = lines.next()
        assert line.startswith('No_of_seqs')

        line = lines.next()
        assert line.startswith('Neff')
        header.n_eff = float(line[value_start_column:].strip())

        line = lines.next()
        assert line.startswith('Searched_HMMs')
        header.searched_hmms = int(line[value_start_column:].strip())

        line = lines.next()
        assert line.startswith('Date')
        value = line[value_start_column:].strip()
        header.date = datetime.datetime.strptime(value, date_pattern)

        line = lines.next()
        assert line.startswith('Command')
        header.command = line[value_start_column:].strip()

        line = lines.next()
        assert len(line.strip()) == 0
        return header

    def _ParseTableOfContents(lines):
        assert lines.next().startswith(' No Hit')
        hits = []
        while True:
            line = lines.next()
            if len(line.strip()) == 0:
                return hits
            hits.append(ParseHeaderLine(line))
        return hits

    def _ParseResultBody(query_id, hits, lines):
        entry_index = None
        query_str = ''
        templ_str = ''
        def _MakeAln(query_id, hit_id, query_string, templ_string,
                     q_offset, t_offset):
            s1 = seq.CreateSequence(query_id, query_string)
            s1.offset = q_offset-1
            s2 = seq.CreateSequence(hit_id, templ_string)
            s2.offset = t_offset-1
            return seq.CreateAlignment(s1, s2)
        try:
            while True:
                line = lines.next()
                if len(line.strip()) == 0:
                    continue
                if line.startswith('Done!'):
                    if len(query_str) > 0:
                        hits[entry_index][0].aln = _MakeAln(\
                            query_id, hits[entry_index][0].hit_id,
                            query_str, templ_str, *hits[entry_index][1])
                    return [h for h, o in hits]
                if line.startswith('No '):
                    if len(query_str) > 0:
                        hits[entry_index][0].aln = _MakeAln(\
                            query_id, hits[entry_index][0].hit_id,
                            query_str, templ_str, *hits[entry_index][1])
                    entry_index = int(line[3:].strip())-1
                    hits[entry_index][0].hit_id = lines.next()[1:].strip()
                    query_str = ''
                    templ_str = ''
                    # skip the next line. It doesn't contain information we
                    # don't already know
                    lines.next()
                    continue
                assert entry_index != None
                if line[1:].startswith(' Consensus'):
                    continue
                if line[1:].startswith(' ss_pred'):
                    continue
                if line[1:].startswith(' ss_conf'):
                    continue
                if line.startswith('T '):
                    end_pos = line.find(' ', 22)
                    assert end_pos != -1
                    templ_str += line[22:end_pos]
                if line.startswith('Q '):
                    end_pos = line.find(' ', 22)
                    assert end_pos != -1
                    query_str += line[22:end_pos]
        except StopIteration:
            if len(query_str) > 0:
                hits[entry_index][0].aln = _MakeAln(query_id,
                                                    hits[entry_index][0].hit_id,
                                                    query_str, templ_str,
                                                    *hits[entry_index][1])
            return [h for h, o in hits]
    header = _ParseHeaderSection(lines)
    # parse the table of contents. This is neccessary as some of the properties
    # (i.e. start of alignment) we need are only given there. From the TOC we
    # create a list of hits that is then further filled with data when we parse
    # the actual result body
    hits = _ParseTableOfContents(lines)
    return header, _ParseResultBody(header.query, hits, lines)

def ParseA3M(a3m_file):
    '''
    Parse secondary structure information and the multiple sequence alignment 
    out of an A3M file.
    
    :param a3m_file: Iterable containing the lines of the A3M file
    :type a3m_file: iterable, e.g. an openend file
    
    :return: Dictionary containing "ss_pred" (:class:`list`), "ss_conf"
             (:class:`list`) and "msa" (:class:`~ost.seq.AlignmentHandle`).
    '''
    profile_dict = dict()
    state = 'NONE'
    pred_seq_txt = ''
    conf_seq_txt = ''
    msa_seq = list()
    msa_head = list()
    for line in a3m_file:
        if len(line.rstrip()) == 0:
            continue
        elif line.startswith('>ss_pred'):
            state = 'sspred'
            continue
        elif line.startswith('>ss_conf'):
            state = 'ssconf'
            continue
        elif line[0] == '>':
            if state == 'ssconf' or state == 'msa':
                msa_seq.append('')
                msa_head.append(line[1:].rstrip())
            else:
                raise IOError('The A3M file is missing the "ss_conf" section')
            state = 'msa'
            continue

        if state == 'sspred':
            pred_seq_txt += line.rstrip()
        elif state == 'ssconf':
            conf_seq_txt += line.rstrip()
        elif state == 'msa':
            msa_seq[len(msa_seq)-1] += line.rstrip()

    profile_dict['ss_pred'] = list()
    profile_dict['ss_conf'] = list()
    for i in range(0, len(pred_seq_txt)):
        profile_dict['ss_pred'].append(pred_seq_txt[i])
        profile_dict['ss_conf'].append(int(conf_seq_txt[i]))
    
    # post processing
    # MSA
    profile_dict['msa'] = None
    if len(msa_seq) > 1:
        t = msa_seq[0]
        al = seq.AlignmentList()
        for i in range(1, len(msa_seq)):
          qs = ''
          ts = ''
          k = 0
          for c in msa_seq[i]:
            if c.islower():
              qs += '-'
              ts += c.upper()
            else:
              qs += t[k]
              ts += c
              k += 1
          nl = seq.CreateAlignment(seq.CreateSequence(msa_head[0], qs), 
                                 seq.CreateSequence(msa_head[i], ts))
          al.append(nl)
        profile_dict['msa'] = seq.alg.MergePairwiseAlignments(\
                                                al,
                                                seq.CreateSequence(msa_head[0],
                                                                   t))
    return profile_dict


def ParseHHM(profile):
    '''Parse secondary structure information and the MSA out of an HHM profile.

    :param profile: Opened file handle holding the profile.
    :type profile: :class:`file`

    :return: Dictionary containing "ss_pred" (:class:`list`), "ss_conf"
             (:class:`list`), "msa" (:class:`~ost.seq.AlignmentHandle`) and
             "consensus" (~ost.seq.SequenceHandle).
    '''
    profile_dict = dict()
    state = 'NONE'
    pred_seq_txt = ''
    conf_seq_txt = ''
    consensus_txt = ''
    msa_seq = list()
    msa_head = list()
    for line in profile:
        if len(line.rstrip()) == 0:
            continue
        if line.rstrip() == '>ss_pred PSIPRED predicted secondary structure':
            state = 'sspred'
            continue
        elif line.rstrip() == '>ss_conf PSIPRED confidence values':
            state = 'ssconf'
            continue
        elif line.rstrip() == '>Consensus':
            state = 'consensus'
            continue
        elif line[0] == '>':
            if state == 'consensus' or state == 'msa':
                msa_seq.append('')
                msa_head.append(line[1:].rstrip())
            else:
                raise IOError('Profile file "%s" is missing ' % profile.name+
                              'the "Consensus" section')
            state = 'msa'
            continue
        elif line[0] == '#':
            state = 'NONE'
            continue

        if state == 'sspred':
            pred_seq_txt += line.rstrip()
        elif state == 'ssconf':
            conf_seq_txt += line.rstrip()
        elif state == 'msa':
            msa_seq[len(msa_seq)-1] += line.rstrip()
        elif state == 'consensus':
            consensus_txt += line.rstrip()

    profile_dict['ss_pred'] = list()
    profile_dict['ss_conf'] = list()
    for i in range(0, len(pred_seq_txt)):
        profile_dict['ss_pred'].append(pred_seq_txt[i])
        profile_dict['ss_conf'].append(int(conf_seq_txt[i]))

    # post processing
    # MSA
    profile_dict['msa'] = None
    if len(msa_seq):
        t = msa_seq[0]
        al = seq.AlignmentList()
        for i in range(1, len(msa_seq)):
            qs = ''
            ts = ''
            k = 0
            for c in msa_seq[i]:
                if c.islower():
                    qs += '-'
                    ts += c.upper()
                else:
                    qs += t[k]
                    ts += c
                    k += 1
            nl = seq.CreateAlignment(seq.CreateSequence(msa_head[0], qs),
                                     seq.CreateSequence(msa_head[i], ts))
            al.append(nl)
        profile_dict['msa'] = seq.alg.MergePairwiseAlignments(\
                                            al,
                                            seq.CreateSequence(msa_head[0], t))
      #print profile_dict['msa'].ToString(80)
    # Consensus
    profile_dict['consensus'] = seq.CreateSequence('Consensus', consensus_txt)

    return profile_dict

def EstimateMemConsumption():
    """
    Estimate the memory needed by HHblits. By default it uses not more than 3G.
    Also for small sequences it already uses quite some memnmory (46AA, 1.48G).
    And since the memory consumption could depend on the iterative search runs,
    how many hits are found in each step, we just go with 4G, here.
    """
    return 4.0, 'G'

class HHblits:
    """
    Initialise a new HHblits "search" for the given query. Query may either
    be a :class:`ost.seq.SequenceHandle` or a string. In the former case, the
    query is the actual query sequence, in the latter case, the query is the
    filename to the file containing the query.

    :param query: Query sequence as file or sequence.
    :type query: :class:`ost.seq.SequenceHandle` or :class:`str`
    :param hhsuite_root: Path to the top-level directory of your hhsuite
                         installation.
    :type hhsuite_root: :class:`str`
    :param hhblits_bin: Name of the hhblits binary. Will only be used if
                        ``hhsuite_root``/bin/hhblits does not exist.
    :type hhblits_bin: :class:`str`
    :param working_dir: Directory for temporary files. Will be created if not
                        present but **not** automatically deleted.
    :type working_dir: :class:`str`
    """
    OUTPUT_PREFIX = 'query_hhblits'
    def __init__(self, query, hhsuite_root, hhblits_bin=None, working_dir=None):
        self.query = query
        self.hhsuite_root = hhsuite_root
        if os.path.exists(os.path.join(self.hhsuite_root, 'bin/hhblits')):
            self.bin_dir = os.path.join(self.hhsuite_root, 'bin')
            self.hhblits_bin = os.path.join(self.hhsuite_root, 'bin/hhblits')
        else:
            self.hhblits_bin = settings.Locate('hhblits',
                                               explicit_file_name=hhblits_bin)
            self.bin_dir = os.path.dirname(self.hhblits_bin)
        self.hhlib_dir = os.path.join(self.hhsuite_root, 'lib', 'hh')
        if working_dir:
            self.needs_cleanup = False
            self.working_dir = working_dir
            if not os.path.exists(working_dir):
                os.mkdir(working_dir)
            if isinstance(query, str):
                self.filename = os.path.abspath(os.path.join(
                    self.working_dir, os.path.basename(query)))
                if self.filename != os.path.abspath(query):
                    shutil.copy(query, self.filename)
            else:
                self.filename = os.path.join(self.working_dir,
                                             '%s.fasta' % HHblits.OUTPUT_PREFIX)
                ost.io.SaveSequence(query, self.filename)
        else:
            self.needs_cleanup = True
            if isinstance(query, str):
                self.working_dir = tempfile.mkdtemp()
                self.filename = os.path.abspath(os.path.join(
                    self.working_dir, os.path.basename(query)))
                shutil.copy(query, self.filename)
            else:
                tmp_dir = utils.TempDirWithFiles((query,))
                self.working_dir = tmp_dir.dirname
                self.filename = tmp_dir.files[0]

    def Cleanup(self):
        """Delete temporary data.

        Delete temporary data if no working dir was given. Controlled by
        :attr:`needs_cleanup`.
        """
        if self.needs_cleanup and os.path.exists(self.working_dir):
            shutil.rmtree(self.working_dir)


    def BuildQueryMSA(self, nrdb, iterations=1, mact=None, cpu=1):
        """Builds the MSA for the query sequence

        This function directly uses hhblits of hhtools. While in theory it
        would be possible to do this by PSI-blasting on our own, hhblits is
        supposed to be faster. Also it is supposed to prevent alignment
        corruption. The alignment corruption is caused by low-scoring terminal
        alignments that draw the sequences found by PSI-blast away from the
        optimum. By removing these low scoring ends, part of the alignment
        corruption can be suppressed. hhblits does NOT call PSIPRED on the MSA
        to predict the secondary structure of the query sequence. This is done
        by addss.pl of hhtools. The predicted secondary structure is stored
        together with the sequences identified by hhblits.

        :param nrdb: Database to be align against; has to be an hhblits database
        :type nrdb: :class:`str`
        :param iterations: Number of hhblits iterations
        :type iterations: :class:`int`
        :param mact: '-mact' of hhblits
        :type mact: :class:`float`
        :param cpu: '-cpu' of hhblits
        :type cpu: :class:`int`
        """
        a3m_file = '%s.a3m' % os.path.splitext(self.filename)[0]
        ost.LogInfo('Using hhblits from "%s"' % self.hhsuite_root)
        full_nrdb = os.path.join(os.path.abspath(os.path.split(nrdb)[0]),
                                 os.path.split(nrdb)[1])
        # create MSA
        hhblits_cmd = '%s -e 0.001 -cpu %d -i %s -oa3m %s -d %s -n %d' % \
                      (self.hhblits_bin, cpu, self.filename, a3m_file,
                       full_nrdb, iterations)
        if mact:
            hhblits_cmd += '-mact %f' % mact
        job = subprocess.Popen(hhblits_cmd, shell=True, cwd=self.working_dir,
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        sout, _ = job.communicate()
        #lines = sout.splitlines()
        #for l in lines:
        #    print l.strip()
        #lines = serr.splitlines()
        #for l in lines:
        #    print l.strip()
        if not os.path.exists(a3m_file):
            ost.LogWarning('Building query profile failed, no output')
            return a3m_file
        # add secondary structure annotation
        addss_cmd = "%s %s" % (os.path.join(self.hhsuite_root,
                                            'lib/hh/scripts/addss.pl'),
                               a3m_file)
        env = dict(os.environ)
        env.update({'PERL5LIB' : os.path.join(self.hhsuite_root,
                                              'lib/hh/scripts'),
                    'HHLIB' : self.hhlib_dir,
                    'PATH' : '%s:%s' % (os.path.join(self.hhsuite_root, 'bin'),
                                        os.environ['PATH'])})
        job = subprocess.Popen(addss_cmd, shell=True, cwd=self.working_dir,
                               env=env, stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE)
        sout, _ = job.communicate()
        lines = sout.splitlines()
        for line in lines:
            if 'error' in line.lower():
                ost.LogWarning('Predicting secondary structure for MSA '+
                               '(%s) failed, on command: %s' % (a3m_file, line))
            return a3m_file
        return a3m_file

    def A3MToProfile(self, a3m_file, hhm_file=None):
        """
        Converts the A3M alignment file to a hhm profile. If hhm_file is not
        given, the output file will be set to <file-basename>.hhm.
        """
        hhmake = os.path.join(self.bin_dir, 'hhmake')
        if not hhm_file:
            hhm_file = '%s.hhm' % os.path.splitext(a3m_file)[0]
        if os.path.exists(hhm_file):
            return hhm_file
        ost.LogVerbose('converting %s to %s' % (a3m_file, hhm_file))
        os.putenv('HHLIB', self.hhlib_dir)
        if subprocess.call('%s -i %s -o %s' % (hhmake, a3m_file, hhm_file),
                           shell=True):
            raise IOError('could not convert a3m to hhm file')
        return hhm_file


    def A3MToCS(self, a3m_file, cs_file=None, options={}):
        """
        Converts the A3M alignment file to a column state sequence file. If
        cs_file is not given, the output file will be set to
        <file-basename>.seq219.
        :param a3m_file: A3M file to be converted.
        :type a3m_file: :class:`str`
        :param cs_file: Output file name (may be omitted)
        :type cs_file: :class:`str`
        :param options: dictionary of options to *cstranslate*, must come with
                        the right amount of '-' in front.
        :type options: :class:`dict`
        """
        cstranslate = os.path.join(self.hhlib_dir, 'bin', 'cstranslate')
        if not cs_file:
            cs_file = '%s.seq219' % os.path.splitext(a3m_file)[0]
        if os.path.exists(cs_file):
            return cs_file
        opt_cmd = list()
        for k, val in options.iteritems():
            if type(val) == type(True):
                if val == True:
                    opt_cmd.append('%s' % str(k))
            else:
                opt_cmd.append('%s %s' % (str(k), str(val)))
        opt_cmd = ' '.join(opt_cmd)
        cs_cmd = '%s -i %s -o %s %s' % (cstranslate, a3m_file, cs_file, opt_cmd)
        ost.LogVerbose('converting %s to %s' % (a3m_file, cs_file))
        job = subprocess.Popen(cs_cmd, shell=True,
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        sout, _ = job.communicate()
        #lines = serr.splitlines()
        #for l in lines:
        #    print l
        lines = sout.splitlines()
        for line in lines:
            if line in 'Wrote abstract state sequence to %s' % cs_file:
                return cs_file
        ost.LogWarning('Creating column state sequence file (%s) failed' % \
                       cs_file)

    def CleanupFailed(self):
        '''In case something went wrong, call to make sure everything is clean.
        '''
        store_needs_cleanup = self.needs_cleanup
        self.needs_cleanup = True
        self.Cleanup()
        self.needs_cleanup = store_needs_cleanup

    def Search(self, a3m_file, database, options={}, prefix=''):
        """
        Searches for templates in the given database. Before running the
        search, the hhm file is copied. This makes it possible to launch
        several hhblits instances at once. Upon success, the filename of the
        result file is returned. This file may be parsed with
        :func:`ParseHHblitsOutput`.
        """
        opts = {'cpu' : 1, # no. of cpus used
                'n'   : 1}   # no. of iterations
        opts.update(options)
        opt_cmd = []
        opt_str = []
        for k, val in opts.iteritems():
            if type(val) == type(True):
                if val == True:
                    opt_cmd.append('-%s' % str(k))
                    opt_str.append(str(k))
            else:
                opt_cmd.append('-%s %s' % (str(k), str(val)))
                opt_str.append('%s%s' % (str(k), str(val)))
        opt_cmd = ' '.join(opt_cmd)
        opt_str = '_'.join(opt_str)
        base = os.path.basename(os.path.splitext(a3m_file)[0])
        hhr_file = '%s%s_%s.hhr' % (prefix, base, opt_str)
        hhr_file = os.path.join(self.working_dir, hhr_file)
        search_cmd = '%s %s -e 0.001 -Z 10000 -B 10000 -i %s -o %s -d %s'%(
            self.hhblits_bin,
            opt_cmd, os.path.abspath(a3m_file),
            hhr_file,
            os.path.join(os.path.abspath(os.path.split(database)[0]),
                         os.path.split(database)[1]))
        ost.LogInfo('searching %s' % database)
        ost.LogVerbose(search_cmd)
        job = subprocess.Popen(search_cmd, shell=True, cwd=self.working_dir,
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        sout, serr = job.communicate()
        if job.returncode != 0:
            lines = sout.splitlines()
            for line in lines:
                print line.strip()
            lines = serr.splitlines()
            for line in lines:
                print line.strip()
            return None
        return hhr_file


__all__ = ['HHblits', 'HHblitsHit', 'HHblitsHeader', 'ParseHeaderLine',
           'ParseHHblitsOutput', 'ParseHHM', 'EstimateMemConsumption']

#  LocalWords:  HHblits MSA hhblits hhtools PSIPRED addss param nrdb str mact
#  LocalWords:  cpu hhm func ParseHHblitsOutput ss pred conf msa
