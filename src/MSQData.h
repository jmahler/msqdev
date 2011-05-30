/*
 * Copyright (C) 2011 Jeremiah Mahler <jmmahler@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <ctime>
#include <string>
#include <iostream>

using namespace std;

/**
 * In the msqdev system each MSQData object represents one
 * chunk of data such as a table.
 * Specific data types are defined elsewhere and inherit this
 * abstract class.
 *
 * Each chunk of data can be represented by data stored in two
 * places: in a file and on the ecu.
 * This explains the need for the readFile(), writeFile(), readEcu(),
 * writeEcu(), and hasChanges() operations.
 * The program which is in control of an object must decide whether
 * the file or the ecu data is valid and which one to update in the
 * event of a difference.
 */
//template <class T>
class MSQData {
	private:
		time_t file_last_modified;
		string name;
		string file_name;

		MSQData() {};  // prevent use of uninitialized constructor
	public:
		virtual ~MSQData() {};

		MSQData(string _name, string _file_name) {
			name = _name;
			file_name = _file_name;
		}

		/**
		 * Get the stored name.
		 *
		 * @returns name
		 */
		string getName() {
			return name;
		}

		/**
		 * Get the stored file name.
		 *
		 * @returns file name
		 */
		string fileName() {
			return file_name;
		}

		/**
		 * Read the data from the ecu and store it internally.
		 *
		 * @returns true on error, false otherwise
		 */
		virtual bool readEcu()=0;

		/**
		 * Read the data from its associated file and store
		 * it internally.
		 */
		virtual void readFile()=0;

		/**
		 * Write the currently stored FILE data to the ecu.
		 *
		 * @returns true on error, false otherwise
		 */
		virtual bool writeEcu()=0;

		/**
		 * Write the currently stored file data to the associated file.
		 */
		virtual void writeFile()=0;

		/**
		 * Copy the ecu data to the file data.
		 * This operation is performed internally and does not
		 * write the data to the file (writeFile()).
		 */
		virtual void cpEcuToFile()=0;

		/**
		 * Are the differences between the file data and the
		 * ecu data?
		 *
		 * @returns true if yes, false otherwise
		 *
		 * The validity of this operations depends on the data being
		 * up to date (see readEcu(), readFile()).
		 */
		virtual bool hasChanges()=0;
};
