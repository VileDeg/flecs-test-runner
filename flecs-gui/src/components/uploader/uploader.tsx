import { useCallback } from "react";
import { useDropzone } from "react-dropzone";



export namespace UnitTest {
  export interface Executed {
    statusMessage: string;
  }
  export interface Passed {
  }
}

// Define test types for clarity
export interface SystemInvocation {
  name: string;
  timesToRun: number;
}

export interface UnitTest {
 
  name: string;
  systems: SystemInvocation[];
  scriptActual: string;
  scriptExpected: string;
}

interface UploaderProps {
  onTestsParsed : (tests: UnitTest[]) => void;
}

// TODO: allow supplying multiple files, merge all files to get a list of the tests
/*
JSON may start with `tests` array or may only contain one test element 
(starts with `name` property of the test)
*/


export const Uploader: React.FC<UploaderProps> = ({ onTestsParsed  }) => {
  const onDrop = useCallback(
    async (acceptedFiles: File[]) => {
      const allTests: UnitTest[] = [];

      for (const file of acceptedFiles) {
        try {
          const text = await file.text();
          const json = JSON.parse(text);

          // Handle both formats:
          // 1) { tests: [ ... ] }
          // 2) { name, systems, ... }
          if (Array.isArray(json.tests)) {
            allTests.push(...json.tests);
          } else if (json.name && json.systems) {
            allTests.push(json);
          } else {
            console.warn(`File ${file.name} has invalid structure`);
          }
        } catch (e) {
          console.error(`Error parsing ${file.name}:`, e);
        }
      }

      // Pass the combined list to the parent
      onTestsParsed(allTests);
    },
    [onTestsParsed]
  );


  const { getRootProps, getInputProps, isDragActive, fileRejections } =
    useDropzone({
      onDrop,
      accept: {
        "application/json": [".json"], // <-- accept only JSON files
      },
      multiple: true,
    });

  return (
    <div
      {...getRootProps()}
      style={{
        border: "2px dashed gray",
        padding: "20px",
        borderRadius: "8px",
        textAlign: "center",
      }}
    >
      <input {...getInputProps()} />
      {isDragActive ? (
        <p>Drop the JSON file here ...</p>
      ) : (
        <p>Drag and drop a JSON file here, or click to select one</p>
      )}

      {fileRejections.length > 0 && (
        <p style={{ color: "red" }}>Only .json files are accepted</p>
      )}
    </div>
  );
};

export default Uploader;
