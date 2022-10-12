module Main where

import System.Environment (getArgs)

fibs :: Integral a => [a]
fibs = 1 : 1 : zipWith (+) fibs (tail fibs)

main :: IO ()
main = getArgs >>= print . (`take` fibs) . read . head
