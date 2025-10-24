import { Uploader, type UnitTest } from "../uploader/uploader.tsx";

import { useState } from 'react'

import {
  ErrorBox,
  TestsList,
  RunButton,
} from "./styles.ts";

import { useFlecsConnection } from "../../context/flecsConnection/useFlecsConnection.ts";
import {UNIT_TEST_COMPONENT_NAME} from "../../common/constants.ts"

interface LandingPageProps {
  onTestsUploaded : () => void;
}

// TODO: allow supplying multiple files, merge all files to get a list of the tests
/*
JSON may start with `tests` array or may only contain one test element 
(starts with `name` property of the test)
*/

export const LandingPage: React.FC<LandingPageProps> = ({ onTestsUploaded }) => {
  const [tests, setTests] = useState<UnitTest[]>([]);
  const [errorMessage, setErrorMessage] = useState<string>("");
  
  const { connection } = useFlecsConnection();

  /*useEffect(() => {
    if (heartbeat) {
      console.log("Heartbeat:", heartbeat);
    }
  }, [heartbeat]);*/
  
  // 🔍 Check backend connection after mount
  // useEffect(() => {
  
  //     });
  
  const onTestsParsed = (tests: UnitTest[]) => {
    setTests(tests);
    runUnitTests(tests);
    onTestsUploaded();
  };
  
  const runTest = async (test: UnitTest) => {
    
    console.log("Running test: ", test);

    const entityName = test.name;
    try {
      // const recv = (responseText : string, url : string) => {
      //   console.log("recv: " + responseText);
      //   console.log("recv: " + url);
      // };
      
      // const err = (responseText : string) => {
        
      //   throw new Error(`Failed to create entity or component: ${responseText}`);
      // };
      
      // TODO: need to wait till request is received? Is it async?
      await connection?.create(entityName);
      console.log("CREATED entity");
      // TODO: what if test with such name already exists?
      // await connection?.add(entityName, UNIT_TEST_COMPONENT_NAME);
      // console.log("CREATED compoennt");
      await connection?.set(entityName, UNIT_TEST_COMPONENT_NAME, test);
      console.log("SET compoennt");

      console.log(`✅ Test "${test.name}" created successfully.`);
    } catch (error: any) {
      console.error("Error running test:", error);
      setErrorMessage(`Error running test "${test.name}": ${error.message}`);
    }
  }
  

  const runUnitTests = async (testsToRun: UnitTest[]) => {
    setErrorMessage("");
    for (const test of testsToRun) {
      // TODO: timeout between running tests?
      await runTest(test);
    }
  };
  return (
    <div>
      {errorMessage && <ErrorBox>{errorMessage}</ErrorBox>}

      <Uploader onTestsParsed={onTestsParsed} />
    
      {tests.length > 0 && (
        <>
          <TestsList>
            {tests.map((t) => (
              <li key={t.name}>
                <strong>{t.name}</strong> – {t.systems.map((s) => s.name).join(", ")}
              </li>
            ))}
          </TestsList>

          <RunButton onClick={() => runUnitTests(tests)}>Run Again</RunButton>
        </>
      )}
    </div>
  );
};
