import styled, { css } from "styled-components";

export const Container = styled.div`
  font-family: "Inter", sans-serif;
  padding: 24px;
  max-width: 800px;
  margin: 0 auto;
  text-align: center;
`;

export const Header = styled.h1`
  font-size: 1.8rem;
  margin-bottom: 16px;
`;

export const StatusBar = styled.div<{ $status: string }>` 
  margin: 16px 0;
  padding: 10px 12px;
  border-radius: 8px;
  font-weight: 500;
  ${({ $status }) =>
    ($status === "Connecting" || $status === "RetryConnecting") &&
    css`
      background-color: #fff3cd;
      color: #856404;
    `}
  ${({ $status }) =>
    $status === "Connected" &&
    css`
      background-color: #d4edda;
      color: #155724;
    `}
  ${({ $status }) =>
    $status === "Disconnected" &&
    css`
      background-color: #f8d7da;
      color: #721c24;
    `}
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

export const TestsList = styled.ul`
  text-align: left;
  background: #f7f7f7;
  padding: 16px;
  border-radius: 8px;
  margin-top: 16px;
  li {
    margin-bottom: 8px;
  }
`;

export const RunButton = styled.button`
  background-color: #007bff;
  color: white;
  border: none;
  padding: 10px 18px;
  border-radius: 6px;
  cursor: pointer;
  font-weight: 500;
  margin-top: 16px;
  transition: background-color 0.2s;
  &:hover {
    background-color: #0056b3;
  }
`;
