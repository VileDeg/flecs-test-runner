// src/pages/ResultsPage.tsx
import React, { useEffect, useState } from "react";
import {
  Container,
  Header,
  Table,
  TableHead,
  TableBody,
  TableRow,
  TableHeader,
  TableCell,
  StatusBadge,
  ErrorBox,
  RefreshButton,
} from "./styles";

import {FLECS_PORT, BASE_URL, UNIT_TEST_COMPONENT_NAME} from "../../common/constants.ts"
import { type UnitTest, type SystemInvocation } from "../uploader/uploader.tsx";

import { useFlecsConnection } from "../../context/flecsConnection/useFlecsConnection.ts";


interface TestResult {
  name: string;
  passed: boolean;
  status: string;
}

export const ResultsPage: React.FC = () => {
  const [tests, setTests] = useState<TestResult[]>([]);
  const [errorMessage, setErrorMessage] = useState("");
  const [isPolling, setIsPolling] = useState(true);

    
  const { connection, status, heartbeat } = useFlecsConnection();

  // Poll every 2 seconds
  useEffect(() => {
    let interval: NodeJS.Timeout;
    const fetchResults = async () => {
      try {
        
        const err = (responseText : string) => {
          throw new Error(`Failed to create entity or component: ${responseText}`);
        };
        
        // Query for entities with UnitTest and Executed components
        const queryParams = {
          // Query entities that have both UnitTest and Executed components
          // Adjust component names as needed for your Flecs world
        };


        connection.query("TestRunner.UnitTest, TestRunner.UnitTest.Executed, ?TestRunner.UnitTest.Passed", queryParams, 
          (results) => {
            console.log("Query results:", results);
            
            // Process the results
            if (results && results.results) {
              const entities = results.results;
              const testResults : TestResult[] = [];

              for (const entity of entities) {
                const components = entity.fields.values;
                const unitTest : UnitTest = components[0];
                const executed : UnitTest.Executed = components[1];
                const passed : UnitTest.Passed = components[2]; // May be undefined if component doesn't exist
                
                testResults.push({
                  name: unitTest.name, 
                  passed: passed !== undefined && passed !== null, // Check if Passed component exists
                  status: executed.statusMessage,
                });
              }

              setTests(testResults);
            } 
          },
          err
        );
        

        // const res = await fetch(`${BASE_URL}/entity?component=${UNIT_TEST_COMPONENT_NAME}`);
        // if (!res.ok) throw new Error(`Server responded with ${res.status}`);
        // const data = await res.json();

        // // Flecs usually returns { results: [ { components... } ] }
        // const entities = data.results ?? data;

        // const parsedTests: TestCase[] = entities.map((e: any) => {
        //   const ut = e.UnitTest as TestCase;
        //   ut.Executed = e.Executed;
        //   ut.Passed = e.Passed;
        //   return ut;
        // });

        
        setErrorMessage("");
      } catch (err: any) {
        setErrorMessage(`Error fetching results: ${err.message}`);
      }
    };

    if (isPolling) {
      fetchResults();
      interval = setInterval(fetchResults, 2000);
    }

    return () => clearInterval(interval);
  }, [isPolling]);

  const togglePolling = () => setIsPolling((prev) => !prev);

  return (
    <Container>
      <Header>Unit Test Results</Header>

      {errorMessage && <ErrorBox>{errorMessage}</ErrorBox>}

      <RefreshButton onClick={togglePolling}>
        {isPolling ? "⏸ Pause Updates" : "▶ Resume Updates"}
      </RefreshButton>

      <Table>
        <TableHead>
          <TableRow>
            <TableHeader>Test Name</TableHeader>
            <TableHeader>Status</TableHeader>
            <TableHeader>Status Message</TableHeader>
          </TableRow>
        </TableHead>
        <TableBody>
          {tests.map((t) => (
            <TableRow key={t.name}>
              <TableCell>{t.name}</TableCell>
              <TableCell>
                {t.passed ? (
                  <StatusBadge $status="passed">Passed</StatusBadge>
                ) : (
                  <StatusBadge $status="failed">Failed</StatusBadge>
                )}
              </TableCell>
              <TableCell>{t.status ?? "-"}</TableCell>
            </TableRow>
          ))}
        </TableBody>
      </Table>
    </Container>
  );
};
