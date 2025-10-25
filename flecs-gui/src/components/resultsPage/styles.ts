import styled, { css } from "styled-components";

export const Container = styled.div`
  font-family: "Inter", sans-serif;
  padding: 24px;
  max-width: 900px;
  margin: 0 auto;
  text-align: center;
`;

export const Header = styled.h1`
  font-size: 1.8rem;
  margin-bottom: 16px;
`;

export const ErrorBox = styled.div`
  background-color: #fdecea;
  color: #b71c1c;
  border-radius: 8px;
  padding: 10px 15px;
  margin-bottom: 16px;
  text-align: left;
  white-space: pre-line;
`;

export const Table = styled.table`
  width: 100%;
  border-collapse: collapse;
  margin-top: 20px;

  th, td {
    border: 1px solid #ccc;
    padding: 8px 10px;
    text-align: left;
  }

  th {
    background-color: #f3f3f3;
  }
`;

export const StatusBadge = styled.span<{ status: "passed" | "failed" | "pending" }>`
  padding: 4px 10px;
  border-radius: 6px;
  font-weight: 600;
  color: white;
  ${({ status }) =>
    status === "passed" &&
    css`
      background-color: #28a745;
    `}
  ${({ status }) =>
    status === "failed" &&
    css`
      background-color: #dc3545;
    `}
  ${({ status }) =>
    status === "pending" &&
    css`
      background-color: #ffc107;
      color: #212529;
    `}
`;

export const RefreshButton = styled.button`
  background-color: #007bff;
  color: white;
  border: none;
  padding: 8px 14px;
  border-radius: 6px;
  cursor: pointer;
  font-weight: 500;
  margin-top: 12px;
  transition: background-color 0.2s;
  &:hover {
    background-color: #0056b3;
  }
`;
