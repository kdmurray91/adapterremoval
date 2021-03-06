/*************************************************************************\
 * AdapterRemoval - cleaning next-generation sequencing reads            *
 *                                                                       *
 * Copyright (C) 2011 by Stinus Lindgreen - stinus@binf.ku.dk            *
 * Copyright (C) 2014 by Mikkel Schubert - mikkelsch@gmail.com           *
 *                                                                       *
 * If you use the program, please cite the paper:                        *
 * S. Lindgreen (2012): AdapterRemoval: Easy Cleaning of Next Generation *
 * Sequencing Reads, BMC Research Notes, 5:337                           *
 * http://www.biomedcentral.com/1756-0500/5/337/                         *
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
\*************************************************************************/
#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <memory>

#include "adapterset.h"
#include "argparse.h"
#include "fastq.h"
#include "alignment.h"
#include "statistics.h"

namespace ar
{

struct alignment_info;


/**
 * Configuration store, containing all user-supplied options / default values,
 * as well as help-functions using these options.
 */
class userconfig
{
public:
	/**
	 * @param name Name of program.
	 * @param version Version string excluding program name.
	 * @param help Help text describing program.
	 */
    userconfig(const std::string& name,
           	   const std::string& version,
           	   const std::string& help);

    /** Parses a set of commandline arguments. */
    argparse::parse_result parse_args(int argc, char *argv[]);

    /** Returns new statistics object, initialized using usersettings. */
    std::auto_ptr<statistics> create_stats() const;


    std::string get_output_filename(const std::string& key, size_t nth = 0) const;


    enum alignment_type
    {
        //! Valid alignment according to user settings
        valid_alignment,
        //! Alignment with negative score
        poor_alignment,
        //! Read not aligned; too many mismatches, not enough bases, etc.
        not_aligned
    };

    /** Characterize an alignment based on user settings. */
    alignment_type evaluate_alignment(const alignment_info& alignment) const;

    /** Returns true if the alignment is sufficient for collapsing. */
    bool is_alignment_collapsible(const alignment_info& alignment) const;

    /** Returns true if the read matches the quality criteria set by the user. **/
    bool is_acceptable_read(const fastq& seq) const;


    /** Trims a read if enabled, returning the #bases removed from each end. */
    fastq::ntrimmed trim_sequence_by_quality_if_enabled(fastq& read) const;


    //! Argument parser setup to parse the arguments expected by AR
    argparse::parser argparser;

    //! Prefix used for output files for which no filename was explicitly set
    std::string basename;
    //! Path to input file containing mate 1 reads (required)
    std::string input_file_1;
    //! Path to input file containing mate 2 reads (for PE reads)
    std::string input_file_2;

    //! Set to true if both --input1 and --input2 are set, or if either of
    //! --interleaved or --interleaved-input are set.
    bool paired_ended_mode;
    //! Set to true if --interleaved or --interleaved-input is set.
    bool interleaved_input;
    //! Set to true if --interleaved or --interleaved-output is set.
    bool interleaved_output;

    //! Character separating the mate number from the read name in FASTQ reads.
    char mate_separator;

    //! The minimum length of trimmed reads (ie. genomic nts) to be retained
    unsigned min_genomic_length;
    //! The maximum length of trimmed reads (ie. genomic nts) to be retained
    unsigned max_genomic_length;
    //! The minimum required overlap before trimming single-end reads.
    unsigned min_adapter_overlap;
    //! The minimum required genomic overlap before collapsing reads into one.
    unsigned min_alignment_length;
    //! Rate of mismatches determining the threshold for a an acceptable
    //! alignment, depending on the length of the alignment. But see also the
    //! limits set in the function 'evaluate_alignment'.
    double mismatch_threshold;

    //! Quality format expected in input files.
    std::auto_ptr<fastq_encoding> quality_input_fmt;
    //! Quality format to use when writing FASTQ records.
    std::auto_ptr<fastq_encoding> quality_output_fmt;

    //! If true, read termini are trimmed for low-quality bases.
    bool trim_by_quality;
    //! The highest quality score which is considered low-quality
    unsigned low_quality_score;

    //! If true, ambiguous bases (N) at read termini are trimmed.
    bool trim_ambiguous_bases;
    //! The maximum number of ambiguous bases (N) in an read; reads exceeding
    //! this number following trimming (optionally) are discarded.
    unsigned max_ambiguous_bases;

    //! If true, PE reads overlapping at least 'min_alignment_length' are
    //! collapsed to generate a higher quality consensus sequence.
    bool collapse;
    // Allow for slipping basepairs by allowing missing bases in adapter
    unsigned shift;

    //! RNG seed for randomly selecting between to bases with the same quality
    //! when collapsing overllapping PE reads.
    unsigned seed;

    //! If true, the program attempts to identify the adapter pair of PE reads
    bool identify_adapters;

    //! The maximum number of threads used by the program
    unsigned max_threads;

    //! GZip compression enabled / disabled
    bool gzip;
    //! GZip compression level used for output reads
    unsigned int gzip_level;

    //! BZip2 compression enabled / disabled
    bool bzip2;
    //! BZip2 compression level used for output reads
    unsigned int bzip2_level;

    //! Maximum number of mismatches (considering both barcodes for PE)
    unsigned barcode_mm;
    //! Maximum number of mismatches (considering both barcodes for PE)
    unsigned barcode_mm_r1;
    //! Maximum number of mismatches (considering both barcodes for PE)
    unsigned barcode_mm_r2;

    adapter_set adapters;

private:
    //! Not implemented
    userconfig(const userconfig&);
    //! Not implemented
    userconfig& operator=(const userconfig&);


    /** Sets up adapter sequences based on user settings.
     *
     * @return True on success, false otherwise.
     */
    bool setup_adapter_sequences();


    //! Sink for --adapter1, adapter sequence expected at 3' of mate 1 reads
    std::string adapter_1;
    //! Sink for --adapter2, adapter sequence expected at 3' of mate 2 reads
    std::string adapter_2;
    //! Sink for --adapter-list; list of adapter #1 and #2 sequences
    std::string adapter_list;

    //! Sink for --barcode-list; list of barcode #1 (and #2 sequences)
    std::string barcode_list;

    //! Sink for user-supplied quality score formats; use quality_input_fmt.
    std::string quality_input_base;
    //! Sink for user-supplied quality score formats; use quality_output_fmt.
    std::string quality_output_base;
    //! Sink for maximum quality score for input / output
    unsigned quality_max;
    //! Sink for the mate separator character; use mate separator
    std::string mate_separator_str;
    //! Sink for --interleaved
    bool interleaved;
};

} // namespace ar

#endif
