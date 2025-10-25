import { Uploader, type TestCase, type TestSystem } from "../uploader/uploader.tsx";

import { useState, useEffect } from 'react'

import {
  Container,
  Header,
  StatusBar,
  ErrorBox,
  TestsList,
  RunButton,
} from "./styles.ts";

import { useFlecsConnection } from "../../context/flecsConnection/useFlecsConnection.ts";

//import {flecs} from "../../flecs";

// Default port 27750

import {FLECS_PORT, UNIT_TEST_COMPONENT_NAME} from "../../common/constants.ts"



interface LandingPageProps {
  onTestsUploaded : () => void;
}

// TODO: allow supplying multiple files, merge all files to get a list of the tests
/*
JSON may start with `tests` array or may only contain one test element 
(starts with `name` property of the test)
*/

export const LandingPage: React.FC<LandingPageProps> = ({ onTestsUploaded }) => {
  const [tests, setTests] = useState<TestCase[]>([]);
  // const [connectionStatus, setConnectionStatus] = useState<
  //   "checking" | "connected" | "failed"
  // >("checking");
  const [connectionStatus, setConnectionStatus] = useState<string>("Disconnected"); // TODO: make more specific (enum strings)
  const [errorMessage, setErrorMessage] = useState<string>("");
  
  const { connection, status, heartbeat } = useFlecsConnection();
  
  useEffect(() => {
    console.log("Flecs status:", status);
    setConnectionStatus(status);
  }, [status]);

  useEffect(() => {
    if (heartbeat) {
      console.log("Heartbeat:", heartbeat);
    }
  }, [heartbeat]);
  
    // 🔍 Check backend connection after mount
  // useEffect(() => {
    
  //     });
    
    
  //   async function checkConnection() {
  //     try {
  //       setConnectionStatus("checking");
  //       const res = await fetch(`${BASE_URL}/health`, { method: "GET" });
  //       if (res.ok) {
  //         setConnectionStatus("connected");
  //       } else {
  //         setConnectionStatus("failed");
  //         setErrorMessage(`Server responded with ${res.status}`);
  //       }
  //     } catch (err: any) {
  //       setConnectionStatus("failed");
  //       setErrorMessage(`Cannot connect to server: ${err.message}`);
  //     }
  //   }

  //   checkConnection();
  // }, []);
  
  const onTestsParsed = (tests: TestCase[]) => {
    setTests(tests);
    runUnitTests(tests);
    onTestsUploaded();
  };
  
  const runTest = async (test: TestCase) => {
    
    /*
    if (this.request.status < 200 || this.request.status >= 300) {
      // Error status
      if (this.err && !this.aborted) {
        this.err(this.request.responseText);
      }
    } else {
      requestOk = true;

      // Request OK
      if (this.recv && !this.aborted) {
        this.recv(this.request.responseText, url);
      }
    }
     */
    
    const entityName = test.name;
    try {
      const recv = (responseText : string, url : string) => {
        console.log("recv: " + responseText);
        console.log("recv: " + url);
      };
      
      const err = (responseText : string) => {
        console.log("err: " + responseText);
        throw new Error(`Failed to create entity or component: ${responseText}`);
      };
      
      connection.create(entityName, recv, err);
      console.log("CREATED entity");
      connection.add(entityName, UNIT_TEST_COMPONENT_NAME, recv, err);
      console.log("CREATED compoennt");
      connection.set(entityName, UNIT_TEST_COMPONENT_NAME, test);
      console.log("SET compoennt");
      
      // // 🧱 Create entity
      // const entityRes = await fetch(`${BASE_URL}/entity/${entityName}`, {
      //   method: "PUT",
      // });

      // if (!entityRes.ok) {
      //   throw new Error(`Failed to create entity: ${entityRes.statusText}`);
      // }

      // ⚙️ Add component
      // const jsonText = JSON.stringify(test);
      // const encoded = encodeURIComponent(jsonText);
      // const url = new URL(`${BASE_URL}/component/${entityName}`);
      // url.searchParams.set("component", UNIT_TEST_COMPONENT_NAME);
      // url.searchParams.set("value", encoded);

      // const compRes = await fetch(url.toString(), {
      //   method: "PUT",
      // });

      // if (!compRes.ok) {
      //   throw new Error(`Failed to create component: ${compRes.statusText}`);
      // }

      console.log(`✅ Test "${test.name}" created successfully.`);
    } catch (error: any) {
      console.error("Error running test:", error);
      setErrorMessage(`Error running test "${test.name}": ${error.message}`);
    }
  }
  

  const runUnitTests = async (testsToRun: TestCase[]) => {
    setErrorMessage("");
    for (const test of testsToRun) {
      // TODO: timeout between running tests?
      await runTest(test);
    }
  };
/**    Connecting:       Symbol('Connecting'),       // Attempting to connect
    RetryConnecting:  Symbol('RetryConnecting'),  // Attempting to restore a lost connection
    Connected:        Symbol('Connected'),        // Connected
    Disconnected:     Symbol('Disconnected'),     // Disconnected (not attempting to */
  return (
    <Container>
      <Header>Unit Test Runner</Header>

      <StatusBar $status={connectionStatus}>
        {(connectionStatus === "Connecting" || connectionStatus === "RetryConnecting") 
            && `Trying to connect to port ${FLECS_PORT}...`}
        {connectionStatus === "Connected" && `✅ Connected to port ${FLECS_PORT}`}
        {connectionStatus === "Disconnected" && "❌ Connection failed"}
      </StatusBar>

      {errorMessage && <ErrorBox>{errorMessage}</ErrorBox>}

      {connectionStatus === "Connected" && (
        <>
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
        </>
      )}
    </Container>
  );
};
