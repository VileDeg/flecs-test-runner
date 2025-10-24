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
  ErrorBox,
  Section,
  SectionHeader,
  EmptyMessage,
} from "./styles";

import { type UnitTest } from "../uploader/uploader.tsx";
import { Timer } from "../Timer/Timer.tsx";

import { useFlecsConnection } from "../../context/flecsConnection/useFlecsConnection.ts";


interface TestResult {
  name: string;
  statusMessage?: string;
}

export const ResultsPage: React.FC = () => {
  const [pendingTests, setPendingTests] = useState<TestResult[]>([]);
  const [passedTests, setPassedTests] = useState<TestResult[]>([]);
  const [failedTests, setFailedTests] = useState<TestResult[]>([]);
  const [errorMessage, setErrorMessage] = useState("");
  const [pendingStartTime, setPendingStartTime] = useState<number | null>(null);
  const [hasTimedOut, setHasTimedOut] = useState(false);

    
  const { connection } = useFlecsConnection();

  // Poll with smart timing and timeout
  useEffect(() => {
    let interval: NodeJS.Timeout;
    const fetchResults = async () => {
      try {
        if (!connection) return;

        // Get pending tests (UnitTest but not Executed)
        const pendingQuery = await connection.query(
          "TestRunner.UnitTest, !TestRunner.UnitTest.Executed", {}
        );
        const newPendingTests = parseQueryResults(pendingQuery, "Not executed yet");

        // Get passed tests (UnitTest with Executed and Passed components)
        const passedQuery = await connection.query(
          "TestRunner.UnitTest, TestRunner.UnitTest.Executed, TestRunner.UnitTest.Passed", {}
        );
        const newPassedTests = parseQueryResults(passedQuery, "Test passed");

        // Get failed tests (UnitTest with Executed but without Passed component)
        const failedQuery = await connection.query(
          "TestRunner.UnitTest, TestRunner.UnitTest.Executed, !TestRunner.UnitTest.Passed", {}
        );
        const newFailedTests = parseQueryResults(failedQuery, "Test failed");

        // If we have pending tests and no start time, set it
        if (newPendingTests.length > 0 && pendingStartTime === null) {
          setPendingStartTime(Date.now());
          setHasTimedOut(false);
        }
        
        // If no pending tests, clear the start time
        if (newPendingTests.length === 0) {
          setPendingStartTime(null);
          setHasTimedOut(false);
        }
        
        // Check for timeout (2 minutes = 120,000ms)
        if (pendingStartTime !== null && Date.now() - pendingStartTime > 120000) {
          setHasTimedOut(true);
          setPendingStartTime(null);
        }

        setPendingTests(newPendingTests);
        setPassedTests(newPassedTests);
        setFailedTests(newFailedTests);
        setErrorMessage("");
      } catch (err: any) {
        setErrorMessage(`Error fetching results: ${err.message}`);
      }
    };

    // Initial fetch
    fetchResults();

    // Only poll if we have pending tests and haven't timed out
    if (!hasTimedOut) { //pendingTests.length > 0 && 
      interval = setInterval(fetchResults, 1000); // 1 second interval
    }

    return () => clearInterval(interval);
  }, [connection, pendingTests.length, pendingStartTime, hasTimedOut]);

  // Helper function to parse query results into TestResult array
  const parseQueryResults = (queryResult: any, defaultStatusMessage: string): TestResult[] => {
    const results: TestResult[] = [];
    
    if (queryResult && queryResult.results) {
      for (const entity of queryResult.results) {
        const components = entity.fields.values;
        const unitTest: UnitTest = components[0];
        const executed: UnitTest.Executed | undefined = components[1];
        
        results.push({
          name: unitTest.name,
          statusMessage: executed?.statusMessage || defaultStatusMessage
        });
      }
    }
    
    return results;
  };

  // Render test table for a specific status
  const renderTestTable = (testList: TestResult[], statusLabel: string) => {
    if (testList.length === 0) {
      return <EmptyMessage>No {statusLabel} tests</EmptyMessage>;
    }

    return (
      <Table>
        <TableHead>
          <TableRow>
            <TableHeader>Test Name</TableHeader>
            <TableHeader>Status Message</TableHeader>
          </TableRow>
        </TableHead>
        <TableBody>
          {testList.map((t) => (
            <TableRow key={t.name}>
              <TableCell>{t.name}</TableCell>
              <TableCell>{t.statusMessage ?? "-"}</TableCell>
            </TableRow>
          ))}
        </TableBody>
      </Table>
    );
  };

  return (
    <Container>
      <Header>Unit Test Results</Header>

      {errorMessage && <ErrorBox>{errorMessage}</ErrorBox>}

      {/* {pendingTests.length > 0 && (
        <Section>
          <SectionHeader $status="pending">
            🟡 Pending Tests ({pendingTests.length})
            {pendingStartTime && (
              <Timer startTime={pendingStartTime} />
            )}
          </SectionHeader>
          {renderTestTable(pendingTests, "pending")}
        </Section>
      )} */}
      
      <Section>
        <SectionHeader $status="pending">
          🟡 Pending Tests ({pendingTests.length})
          
        </SectionHeader>
        {renderTestTable(pendingTests, "pending")}
      </Section>

      <Section>
        <SectionHeader $status="passed">
          ✅ Passed Tests ({passedTests.length})
        </SectionHeader>
        {renderTestTable(passedTests, "passed")}
      </Section>

      <Section>
        <SectionHeader $status="failed">
          ❌ Failed Tests ({failedTests.length})
        </SectionHeader>
        {renderTestTable(failedTests, "failed")}
      </Section>

      {hasTimedOut && (
        <ErrorBox>Test execution timed out after 2 minutes</ErrorBox>
      )}
    </Container>
  );
};
