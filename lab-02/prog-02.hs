module Main where

divisors :: Integral a => a -> [a]
divisors n = [k | k <- [1 .. n], n `mod` k == 0]

isPrime :: Integral a => a -> Bool
isPrime n = length (takeWhile (<= w n) $ divisors n) == 1
  where
    w = ceiling . sqrt . fromIntegral

main :: IO ()
main = do
  putStrLn "Input a number:"
  n <- read <$> getLine
  putStrLn $
    if isPrime n
      then "It's prime!"
      else "It isn't..."
